#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Instrucciones.h"
#include "MaquinaVirtual.h"

int VerificarHeader (int header[6]);
void LeerBinario(MV* maquina, const char* path);
void EjecutarBinario(MV* maquina);
void EjecutarInstruccion(MV* maquina, int instruccion);
int GetBytes(int num, int i, int n);
int GetBits(int num, int i, int n);
void cargarRegistros(int *, MV*);
int VerificarMemoria(int *);
void CargarVDD(char *, MV*, int *);
void iniciaDiscos(MV*);

//DEVUELVE LA POSICION A ACCEDER EN EL VDD
int posicionVDD(int h, int c, int s,VDD disco);



int main(int argc, char** argv){
    MV maquina;
    maquina.flag_b = 0;
    maquina.flag_c = 0;
    maquina.flag_d = 0;
    maquina.flag_break = 0;
    maquina.registros[IP] = 0;
    int idDiscos=0;
    if(argc > 2){
        iniciaDiscos(&maquina);
        for(int i = 2; i<argc;i++){
            if(strstr(argv[i],".vdd")){
                    CargarVDD(argv[i],&maquina,&idDiscos);
            }else if(strcmp(argv[i], "-b") == 0){
                maquina.flag_b = 1;
            }else if(strcmp(argv[i], "-c") == 0){
                maquina.flag_c = 1;
            }else if(strcmp(argv[i], "-d") == 0){
                maquina.flag_d = 1;
            }else{
                printf("%s no es un flag valido.", argv[i]);
            }
        }
    }
    LeerBinario(&maquina, argv[1]);
    EjecutarBinario(&maquina);


}

//Lee el archivo binario cargando todas las instrucciones en la memoria de la maquina y
// determina el valor del registro DS en base a la cantidad de instrucciones.
void LeerBinario(MV* maquina, const char* path){
    FILE* archivo = fopen(path, "rb");
    if(!archivo){
        printf("Error abriendo archivo binario.");
        return;
    }
    int aux = 0;
    int header[6];
    fread(header, sizeof(header[0]), 6, archivo);

    if(!VerificarHeader(header)){
        printf("El formato de archivo es invalido");
        return;
    }

    if (VerificarMemoria(header)){
        cargarRegistros(header,maquina);
        //Carga instrucciones
        for(int i = 0; i<header[4]; i++){
            fread(&aux, sizeof(aux), 1, archivo);
            maquina->memoria[i] = aux;
        }
    } else{
        printf("El proceso no puede ser cargador, memoria insuficiente");
    }

    //maquina->registros[DS] = header[1];
    fclose(archivo);
}


void EjecutarBinario(MV* maquina){
    while(maquina->registros[IP]< maquina->registros[DS] ){ //mientras no deba detenerse y el ip apunte a una instruccion ejecutar una instruccion.
        fflush(stdin);
        EjecutarInstruccion(maquina, maquina->memoria[maquina->registros[IP]]);  //Ejecuta la instruccion en la celda de memoria a la que apunta el IP.
        printf("EBX: %x \n", maquina->registros[B]);
        if(maquina->flag_break){
            Breakpoint(maquina);
        }
    }
}

void EjecutarInstruccion(MV* maquina, int instruccion){
     int tipo, tipo2;
    int auxvalor, auxvalor2;
    int valor, valor2;



   // printf("INSTRUCCION: %08X \n", instruccion);
    int primeros_dos_bytes = GetBytes(instruccion, 0,2);
    int codigo_instruccion;
    if(primeros_dos_bytes == 0xFF){
        codigo_instruccion = GetBytes(instruccion,2,1);
        //printf("CODIGO0: %d\n", codigo_instruccion);
        Instrucciones_0[codigo_instruccion-1](maquina);
    }else if(primeros_dos_bytes>=0xF0)                //1 OP
    {
        codigo_instruccion = GetBytes(instruccion,1,1);

        tipo=GetBits(instruccion,8,2);
        auxvalor=GetBits(instruccion,16,16);
        if (tipo==0) //inmediato, puede ser neg
        {
         if (auxvalor>=0x8000)  //si es mayor o igual a 0x8000 es negativo
         {
          valor = ~0x0000FFFF;
          valor = valor | (auxvalor & 0x0000FFFF);
          //printf("%0X \n", valor);
         //printf("%d \n", valor);
         }
         else
            valor = auxvalor;
        }
         else
            valor = auxvalor;
        Instrucciones_1[codigo_instruccion](maquina,valor,tipo);
    }
    else{ //2 OP
        codigo_instruccion = GetBytes(instruccion,0,1);

        tipo=GetBits(instruccion,4,2);
        auxvalor=GetBits(instruccion,8,12);
        tipo2=GetBits(instruccion,6,2);
        auxvalor2=GetBits(instruccion,20,12);
        if (tipo==0) //inmediato, puede ser neg
        {
         if (auxvalor>=0x800)  //si es mayor o igual a 0x800 es negativo
         {
          valor = ~0x00000FFF;
          valor = valor | (auxvalor & 0x00000FFF);
         }
         else
            valor = auxvalor;
        }
        else
            valor = auxvalor;
        if (tipo2==0x00) //inmediato, puede ser neg
        {
         if (auxvalor2>=0x800)  //si es mayor o igual a 0x800 es negativo
         {
          valor2 = ~0x00000FFF;
          valor2 = valor2 | (auxvalor2 & 0x00000FFF);
         }
         else
            valor2 = auxvalor2;
        }
         else
            valor2 = auxvalor2;
        Instrucciones_2[codigo_instruccion](maquina, valor, valor2, tipo, tipo2);
    }
}


int GetBytes(int num, int i, int n){
    return GetBits(num, i*4, 4*n);
}

int GetBits(int num, int i, int n){
    int tmp = num>>(32-(i+n));
    int mask = (((int) 1) << n) - 1;
    return tmp&mask;
}

int VerificarHeader (int header[6]){
    char* aux;
    aux = (char*)(&header[0]);
    if(aux[0]!='M'||aux[1]!='V'||aux[2]!='-'||aux[3]!='2'){
        return 0;
    }
    aux = (char*)(&header[5]);
    if(aux[0]!='V'||aux[1]!='.'||aux[2]!='2'||aux[3]!='2'){
        return 0;
    }

    return 1;

}

int VerificarMemoria(int header[6]){
    if(header[1]+header[2]+header[3]>CELDAS_MEMORIA-header[4])
        return 0;
    return 1;
}

void cargarRegistros(int header[6], MV* maquina){       //registro= tamaño(parte alta) y posicion(parte baja)
        //iniciacion de los registros DS,ES,SS y CS
       maquina->registros[CS]=(header[4]<<16); //CS=0x300A0000 ---> 0x0000300A
       maquina->registros[DS]=(header[1]<<16)|(maquina->registros[CS]>>16 & 0x0000FFFF);//DS=0x00190000 | CS=0xFFFFA000 --> DS=0x0019000A
       maquina->registros[ES]=(header[3]<<16)|(maquina->registros[DS]>>16 & 0x0000FFFF);
       maquina->registros[SS]=(header[2]<<16)|(maquina->registros[ES]>>16 & 0x0000FFFF);

       maquina->registros[HP]=0x00020000;
       maquina->registros[IP]=0x00030000;
       maquina->registros[SP]=0x00010000 | (maquina->registros[SS]>>16 & 0x0000FFFF);
       maquina->registros[BP]=0x00010000; //parte baja no especificada
}

void iniciaDiscos(MV* maquina){
    for (int i=0; i<CANT_DISCOS;i++)
        strcpy(maquina->discos[i].nomArch,"");
}

void generaGUID(char* guid){
    int i,n=34;
    char posiblesCar[17]="0123456789abcdef";
    int random;
    for(i=0;i<=n;i++){
        random=rand()%16;
        guid[i]=posiblesCar[random];
    }
}

void CargarVDD(char* nomArch, MV* maquina, int* i){ //nombre del archivo como parametro
    FILE* archivo;
    archivo=fopen(nomArch,"rb+");
    if(!archivo){
        //SI NO EXISTE SE CREA EL DISCO
        archivo=fopen(nomArch,"wb");

        //ESCRITURA DEL HEADER
        int id= 0x56444430;
        int version=1;
        char GUID[34];
        generaGUID(GUID);
        char fechayhora[8];
        char* fecha;
        char* hora;

        int tipo=1;
        int tamanio=128;
        int sector=512;
        int relleno=0;

        //AGREGA FECHA Y HORA ACTUAL
        time_t t;
        struct tm *tm;
        t=time(NULL);
        tm=localtime(&t);
        strftime(fechayhora, 20, "%d%m%Y,%H%M%S,", tm);
        fecha=strtok(fechayhora,",");
        hora=strtok(NULL,",");

        fwrite(&id,sizeof(int),1,archivo); //id
        fwrite(&version,sizeof(int),1,archivo);//version
        fwrite(&GUID,sizeof(int)*4,1,archivo);            //sizeof(char) para que lea 1byte
        fwrite(&fecha,sizeof(int),1,archivo);//fecha
        fwrite(&hora,sizeof(int),1,archivo);//hora
        fwrite(&tipo,sizeof(char),1,archivo);//tipo
        fwrite(&tamanio,sizeof(char),1,archivo);//cilindros
        fwrite(&tamanio,sizeof(char),1,archivo);//cabezas
        fwrite(&tamanio,sizeof(char),1,archivo);//sectores
        fwrite(&sector,sizeof(int),1,archivo);//tamaño del sector
        fwrite(&relleno,sizeof(char)*472,1,archivo);//relleno
    }

    //AVANZA EN EL ARCHIVO BINARIO HASTA LOS DATOS QUE QUEREMOS
    fseek(archivo,33,SEEK_SET);

    //LECTURA DE HEADER
    fread(&(maquina->discos[*i].cilindros), sizeof(char), 1, archivo);
    fread(&(maquina->discos[*i].cabezas), sizeof(char), 1, archivo);
    fread(&(maquina->discos[*i].sectores), sizeof(char), 1, archivo);
    fread(&(maquina->discos[*i].tSector), sizeof(int), 1, archivo);

    maquina->discos[*i].header=512;
    maquina->discos[*i].ID=*i;
    strcpy(maquina->discos[*i].nomArch,nomArch);
    maquina->discos[*i].estado=0x00;
    (*i)++;
    fclose(archivo);
}



