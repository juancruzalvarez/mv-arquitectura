#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "Instrucciones.h"

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)
#define ES_DIGITO(a) ((a>=48)&&(a<=57))

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #define ClearScreen() system("clear");
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define ClearScreen() system("cls");
#endif



void MostrarRangoMemoria(MV maquina, int mem_start, int mem_end){
    for(int i = MAX(0,mem_start);i<=MIN(CELDAS_MEMORIA-1, mem_end);i++){
       printf("%s[%d] %x\n", i == maquina.registros[IP]? ">": " ", i, maquina.memoria[i]);
    }
}

int EncontrarChar(char* str, int str_len, char c){
    for(int i = 0; i<str_len; i++){
        if(str[i] == c)
            return i;
    }
    return -1;
}

int GetNum(char* str, int str_len){
    int n = 0;
    for(int i = str_len-1; i>=0; i--){
        if(!ES_DIGITO(str[i])){
            return -1;
        }
        n += (str[i]-48)*pow(10, str_len-1-i);
    }
    return n;
}

void Disassemble(MV maquina){
    MostrarRangoMemoria(maquina, maquina.registros[IP]-5, MIN(maquina.registros[IP]+5, maquina.registros[DS]));
    printf("DS:  %d\n", maquina.registros[DS]);
    printf("IP:  %d\n", maquina.registros[IP]);
    printf("CC: #%x\n", maquina.registros[CC]);
    printf("AC: #%x\n", maquina.registros[AC]);
    printf("A:  #%x\n", maquina.registros[A]);
    printf("B:  #%x\n", maquina.registros[B]);
    printf("C:  #%x\n", maquina.registros[C]);
    printf("D:  #%x\n", maquina.registros[D]);
    printf("E:  #%x\n", maquina.registros[E]);
    printf("F:  #%x\n", maquina.registros[F]);

}

typedef enum TIPO_OPERADOR {
    INMEDIATO = 0,
    REGISTRO,
    MEMORIA
}TIPO_OPERADOR;

int GetValor(MV* maquina, int op, int tipo) {
    int res, aux;
    switch(tipo){

        case INMEDIATO: {
            res = op;
            break;
        }
        case MEMORIA: {
            res = maquina->memoria[maquina->registros[DS] + op];
            break;
        }
        case REGISTRO: {
            if (op > 0x30){ //X
                aux = maquina->registros[op-0x30];
                aux = aux & 0xFFFF;
                if (aux >= 0x8000){
                    res = ~0xFFFF;
                    res = res | (aux & 0xFFFF);
                }else {
                    res = aux;
                }
            }else if (op > 0x20){ //H
                aux = maquina->registros[op-0x20];
                aux = (aux & 0xFF00) >> 8;
                if (aux >= 0x80){
                    res = ~0xFF;
                    res = res | (aux & 0xFF);
                }else{
                    res = aux;
                }
            }else if (op > 0x10){ //L
                aux = maquina->registros[op-0x10];
                aux = aux & 0xFF;
                if (aux>=0x80){
                      res = ~0xFF;
                      res = res | (aux & 0xFF);
                }else{
                     res = aux;
                }
            }else{  //EX (registro entero)
                res = maquina->registros[op];
            }
            break;
        }

    }

    return res;
}

void SetValor(MV* maquina, int op, int tipo, int valor) {
    switch(tipo){
        case MEMORIA:{
            maquina->memoria[maquina->registros[DS] + op] = valor;
            break;
        }
        case REGISTRO:{
            if (op > 0x30){
                maquina->registros[op-0x30] = maquina->registros[op-0x30] &  ~0xFFFF;
                maquina->registros[op-0x30] = maquina->registros[op-0x30] | (valor & 0xFFFF);
            }else if (op > 0x20){
                valor = valor << 8;
                maquina->registros[op-0x20]= maquina->registros[op-0x20] &  ~0xFF00;
                maquina->registros[op-0x20] = maquina->registros[op-0x20] | (valor & 0xFF00);
            }else if (op > 0x10){
                maquina->registros[op-0x10] = maquina->registros[op-0x10] &  ~0xFF;
                maquina->registros[op-0x10] = maquina->registros[op-0x10] | (valor & 0xFF);
            }else{
                maquina->registros[op]=valor;
            }
            break;
        }
    }
}


void ActualizarCC(MV* maquina, int res){
    if(res > 0)       //positivo
        maquina->registros[CC] = 0x00000000;
    else if(res < 0)  //negativo
        maquina->registros[CC] = 0x80000000;
    else              //cero
        maquina->registros[CC] = 0x00000001;
}

int BitN(int cc){
    return (cc & 0x80000000);
}

int BitZ(int cc){
    return (cc & 0x1);
}

//2 OPERANDOS
void MOV (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("Move %d %d %d %d\n", op1, op2, tipo1, tipo2);
    SetValor(maquina, op1, tipo1, GetValor(maquina, op2, tipo2));
    maquina->registros[IP]++;
}

void ADD (MV* maquina, int op1, int op2, int tipo1, int tipo2){
  printf("add %d %d %d %d\n " , op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina,op1,tipo1) + GetValor(maquina,op2,tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void SUB (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("sub %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina,op1,tipo1) - GetValor(maquina,op2,tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void MUL (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("mul %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina,op1,tipo1) * GetValor(maquina,op2,tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void DIV (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("div %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux, dividendo = GetValor(maquina, op1, tipo1), divisor = GetValor(maquina,op2,tipo2);
    if (divisor != 0){ //en caso contrario hace algo?
        aux = dividendo/divisor;
        SetValor(maquina, op1, tipo1, aux);
        SetValor(maquina, AC, REGISTRO, dividendo%divisor); //resto
        ActualizarCC(maquina, aux);
    }
    maquina->registros[IP]++;
}

void SWAP (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("swap %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1);
    SetValor(maquina, op1, tipo1, GetValor(maquina, op2, tipo2));
    SetValor(maquina, op2, tipo2, aux);
    maquina->registros[IP]++;
}

void CMP (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("cmp %d %d %d %d \n", op1, op2, tipo1, tipo2);
    ActualizarCC(maquina, GetValor(maquina,op1,tipo1) - GetValor(maquina,op2,tipo2));
    maquina->registros[IP]++;
}

void AND (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("and %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) & GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void OR (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("or %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) | GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void XOR (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("xor %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) ^ GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void SHL (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    printf("shl %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) << GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

//ver lo del signo
void SHR (MV* maquina, int op1, int op2, int tipo1, int tipo2){
    int aux = GetValor(maquina, op1, tipo1) >> GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

//1 OPERANDO

void JMP (MV* maquina, int op1, int tipo1){
    printf("jmp %d %d  \n", op1, tipo1);
     maquina->registros[IP] = GetValor(maquina, op1, tipo1);
}

void JZ (MV* maquina, int op1, int tipo1){
    printf("jz %d %d  \n", op1,tipo1);
    if (BitZ(maquina->registros[CC])) //bit cero en 1
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JP (MV* maquina, int op1, int tipo1){
    printf("jp %d %d  \n", op1,tipo1);
    if (!BitN(maquina->registros[CC]) && !BitZ(maquina->registros[CC])) //bit signo en 0 y bit cero en 0
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JN (MV* maquina, int op1, int tipo1){
    printf("jp %d %d  \n", op1,tipo1);
    if (BitN(maquina->registros[CC])) //bit signo en 1
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JNZ (MV* maquina, int op1, int tipo1){
    printf("jnz %d %d  \n", op1,tipo1);
    if (!BitZ(maquina->registros[CC])) //bit cero en 0
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JNP (MV* maquina, int op1, int tipo1){
    printf("jnp %d %d \n", op1, tipo1);
    if (BitN(maquina->registros[CC]) || BitZ(maquina->registros[CC])) //no positivo
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JNN (MV* maquina, int op1, int tipo1){
    printf("jnn %d %d \n", op1, tipo1);
    if (!BitN(maquina->registros[CC]) || BitZ(maquina->registros[CC])) //no negativo
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void LDH (MV* maquina, int op1, int tipo1){
    printf("ldh %08x %d  \n", op1,tipo1);
    int aux;
    if (tipo1==0x01){
        aux = GetValor(maquina,op1,tipo1) & 0xFFFF; //registro tiene 4 bytes, guardo solo los ultimos 2.
    }else if (tipo1==0x02){
        aux = maquina->memoria[maquina->registros[DS]+op1] & 0xFFFF; //celda tiene 4 bytes, guardo solo los ultimos 2.
    }else {
        aux  = op1<<16;//<< 16; // op inmediato ya es de 16 bits (2bytes)
    }
    maquina->registros[AC] = (maquina->registros[AC] & ~0xFFFF0000);
    maquina->registros[AC] = maquina->registros[AC] | (aux & 0xFFFF0000); //cambio primeros dos bytes de AC q tiene 4 bytes
    maquina->registros[IP]++;
}

void LDL (MV* maquina, int op1, int tipo1){
    printf("ldl %d %d  \n", op1,tipo1);
    int aux;
    if (tipo1==0x01){
        aux = GetValor(maquina,op1,tipo1) & 0xFFFF; //registro tiene 4 bytes, guardo solo los ultimos 2.
    }else if (tipo1==0x02){
        aux = maquina->memoria[maquina->registros[DS]+op1] & 0xFFFF; //celda tiene 4 bytes, guardo solo los ultimos 2.
    }else {
        aux  = op1;//<< 16; // op inmediato ya es de 16 bits (2bytes)
    }
    maquina->registros[AC]= (maquina->registros[AC] & ~0xFFFF);
    maquina->registros[AC]= maquina->registros[AC] | (aux & 0xFFFF); //cambio ULT dos bytes de AC q tiene 4 bytes
    maquina->registros[IP]++;
}

void NOT (MV* maquina, int op1, int tipo1){
    printf("not %d %d  \n", op1,tipo1);
    int aux = ~GetValor(maquina,op1,tipo1);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void STOP (MV* maquina){
    printf("stop");
    maquina->registros[IP] = maquina->registros[DS]+1;
}

void RND (MV* maquina, int op1, int tipo1){
    printf("rnd %d %d \n", op1, tipo1);
    srand(time(0)); //hace que varie en cada ejecucion del programa, sino siempre es la misma secuencia
    SetValor(maquina, AC, REGISTRO, rand() % (GetValor(maquina,op1,tipo1)+1));
    maquina->registros[IP]++;
}

void SYS (MV* maquina, int op1, int tipo1){
    printf("Sys \n");

    switch(op1){
        case 0x1:{
        //printf("read\n");
            //read
            int mem_start = GetValor(maquina, D, REGISTRO);
            int n = GetValor(maquina, 0x3C, REGISTRO);
            int config = GetValor(maquina, 0x3A, REGISTRO);
            int write_prompt = !(config&0x800);
            int aux=0;

            if(write_prompt && config&0x100){
                printf("[%04d] ", mem_start);
            }
            for(int i = 0; i<n; i++){

                if(config&0x100){
                    aux = getchar();
                    if(aux == '\n' || aux == '\0')
                        break;
                }else{
                    if(write_prompt){
                        printf("[%04d] ", mem_start+i);
                    }
                    scanf( (config&0x008) ? "%x" : ((config&0x004)?"%o" : "%d" ), &aux);
                }
                SetValor(maquina, mem_start+i, MEMORIA, aux);
            }
            break;
        }

        case 0x2:{
            //printf("write\n");
            //write
            int mem_start = GetValor(maquina, D, 0x01);
            int n = GetValor(maquina, 0x3C, 0x01);
            int config = GetValor(maquina, 0x3A,0x01);
            int write_prompt = !(config&0x800);
            int new_line = !(config&0x100);
            int val = 0;
            for(int i = 0; i<n; i++){
                val = GetValor(maquina, mem_start+i, MEMORIA);
                if(write_prompt){
                    printf("[%04d] ", mem_start+i);
                }

                config & 0x10 ? printf("%c ", val & 0xFF) : printf(" ");
                config & 0x08 ? printf("%%%X ", val) : printf(" ");
                config & 0x04 ? printf("@%o ", val) : printf(" ");
                config & 0x01 ? printf("%d ", val) : printf(" ");
                if(new_line)
                    printf("\n");

            }

            break;
        }

        case 0xF:{
            //breakpoint
            if(maquina->flag_b){
                maquina->flag_break = 1;
            }

            break;
        }
    }
    maquina->registros[IP]++;
}


void Breakpoint(MV* maquina){
    maquina->flag_break = 0;
    if(maquina->flag_c){
        ClearScreen();
    }
    if(maquina->flag_d){
        Disassemble(*maquina);
    }
    printf("[%04d] cmd: ", maquina->registros[IP]);
    char buffer[10];  //maxima cantidad de caracteres validos es direccion de 4 digitos, espacio , otra direccion de 4 digitos y /n (10).
    fgets (buffer, 10, stdin);
    int nl = EncontrarChar(buffer, 10, '\n');
    if(nl == -1){
        printf("Comando invalido. \n");
    }else{
        if(nl==1){
            if(buffer[0] == 'p'){
                maquina->flag_break = 1;
            }else if(buffer[0] != 'r'){
                printf("Comando invalido. \n");
            }
        }else{
            int mem_start = -1, mem_end = -1;
            int espacio = EncontrarChar(buffer,10, ' ');
            if(espacio == -1){
                mem_start = GetNum(buffer, nl);
                mem_end = mem_start;
            }else{
                mem_start = GetNum(buffer, espacio);
                mem_end =   GetNum(&buffer[espacio+1], nl-espacio-1);
            }
            if(mem_start == -1 || mem_end == -1){
                printf("Comando invalido. \n");
            }else{
                MostrarRangoMemoria(*maquina, mem_start, mem_end);
            }
        }
    }
}



F0 Instrucciones_0 [1]  = {&STOP};
F1 Instrucciones_1 [12] = {&SYS, &JMP, &JZ, &JP, &JN, &JNZ, &JNP, &JNN, &LDL, &LDH, &RND, &NOT};
F2 Instrucciones_2 [12] = {&MOV, &ADD, &SUB, &SWAP, &MUL, &DIV, &CMP, &SHL, &SHR, &AND, &OR, &XOR};