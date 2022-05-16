#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "error.h"
#include "traductor.h"
#include "parser.h"

#define EXTENSION_ASM     ".asm"  // extension del archivo que contiene el codigo asm.
#define EXTENSION_BINARIO ".mv2"  // extension del archivo binario generado.
#define DEFAULT_SEG_SIZE  1024    // tama�o por defecto de cada uno de los segmentos(menos cs).
#define MAX_SEG_SIZE      0xFFFF  // maximo tamaño permitido para los segmnetos.
#define MAX_LINE_LEN      10000   // cantidad maxima de caracteres por linea en el codigo asm.     ??
#define MAX_PROG_LEN      10000   // cantidad maxima de instrucciones que puede tener un programa. ??

/*
 TODO:
ok- argumentos (dejar para el final, asi es mas facil de probar)
ok- escribir binario
ok- hacer que parser lea todo en mayusc
usar free_line del parser luego de leer??
ok- verificar los tama�os de los segmentos
*/

typedef struct segmentos_t
{
   int ds, // tama�o data seg
       ss, // tama�o stack seg
       es, // tama�o extra seg
       cs; // tama�o code seg
} segmentos_t;


// lee los simbolos(constants, rotulos) y lee las directivas de tama�o de segmento.
// devuelve la cantidad de instrucciones leidas
int leer_simbolos(FILE *archivo, smb_list_t *simbolos, segmentos_t *segmentos);

// lee el archivo y traduce todas las instrucciones encontradas.
// dependiendo de la variable out, escribe o no a consola la traduccion.
// devuelve true si no hubo errores, sino false
bool traducir(FILE *archivo, smb_list_t simbolos, int *instrucciones, bool out);

// recorre la lista de simbolos, y cuando encuentra una constante string,
// remplaza su valor por la direccion de memoria y guarda en la memoria el valor de la string.
// devuelve el valor final del code segment:
//    cant_instrucciones + cantidad de celdas usadas para guardar strings.
int resolver_simbolos(smb_list_t simbolos, int cant_instrs, int *instrucciones);

// muestra los valores del programa luego de leer y resolver simbolos.
// muestra tama�o de los segmentos, lista de simbolos con sus respectivos valores,
// cantidad de instrucciones del programa, y la memoria destinada a almacenar las constantes string.
void debug_simbolos(smb_list_t simbolos, segmentos_t seg, int *instrucciones, int nro_instrs);

int main(int argc, char **argv)
{
   char *path_archivo_asm = NULL, *path_archivo_bin = NULL;
   bool write = FALSE, out = FALSE; 
   smb_list_t simbolos = NULL;
   segmentos_t segmentos = {DEFAULT_SEG_SIZE, DEFAULT_SEG_SIZE, DEFAULT_SEG_SIZE, DEFAULT_SEG_SIZE};
   int instrucciones[MAX_PROG_LEN];
   int nro_instrs = -1;

   //leer argumentos del programa.
   for(int i  = 1; i<argc; i++){
      if(strcmp(argv[i], "-o") == 0){
         out = TRUE;
      }else{
         int n = strlen(argv[i]);
         if(n<4){
            printf("%s no es un argumento valido.\n",argv[i]);
         }
         char* extension = &(argv[i][n-4]);
         if(strcmp(extension, EXTENSION_ASM) == 0)
            path_archivo_asm = argv[i];
         else if(strcmp(extension, EXTENSION_BINARIO) == 0)
            path_archivo_bin = argv[i];
         else{
            printf("%s no es un argumento valido.\n",argv[i]);
         }
      }
   }

   if(!path_archivo_asm || !path_archivo_bin){
      printf("Error: no fueron especificados los archivos.\n");
      exit(-1);
   }
   FILE *archivo_asm = fopen(path_archivo_asm, "rt");
   if (!archivo_asm)
   {
      printf("No se pudo abrir el archivo asm.\n");
      exit(-1);
   }
   nro_instrs = leer_simbolos(archivo_asm, &simbolos, &segmentos);
   segmentos.cs = resolver_simbolos(simbolos, nro_instrs, instrucciones);
   rewind(archivo_asm);
   write = traducir(archivo_asm, simbolos, instrucciones, out);
   fclose(archivo_asm);
   if (write)
   {
      // escribir binario
      FILE* archivo_bin = fopen(path_archivo_bin, "wb");
      if (!archivo_bin)
      {
         printf("No se pudo crear el archivo binario.\n");
         exit(-1);
      }
      //escribir header
      fwrite("MV-2", sizeof(char), 4, archivo_bin);
      fwrite(&segmentos, sizeof(segmentos_t), 1, archivo_bin);
      fwrite("V.22", sizeof(char), 4, archivo_bin);
      fwrite(&instrucciones, sizeof(int), segmentos.cs, archivo_bin);
   }
}

int leer_simbolos(FILE *archivo, smb_list_t *simbolos, segmentos_t *segmentos)
{
   char linea[MAX_LINE_LEN];
   int nro_instruccion = 0;
   while (fgets(linea, MAX_LINE_LEN, archivo))
   {
      char **parsed = parse_line(linea);
      if (parsed[CONST] && parsed[CONST_VAL])
      {
         agregar_simbolo(simbolos, (smb_t){parsed[CONST], parsed[CONST_VAL]});
      }

      if (parsed[SEG] && parsed[SEG_SIZE])
      {
         int seg_size = atoi(parsed[SEG_SIZE]);
         if(seg_size>MAX_SEG_SIZE){
            printf("Error: el maximo tamaño de segmento permitido es %d.\n", MAX_SEG_SIZE);
         }else if(seg_size<=0){
            printf("Error: el tamaño de segmento debe ser un numero positivo.\n");
         }

         if (strcmp(parsed[SEG], "DATA") == 0)
         {
            segmentos->ds = seg_size;
         }
         else if (strcmp(parsed[SEG], "EXTRA") == 0)
         {
            segmentos->es = seg_size;
         }
         else if (strcmp(parsed[SEG], "STACK") == 0)
         {
            segmentos->ss = seg_size;
         }
         else
         {
            printf("Error: %s no es el nombre de ningun segmento.\n", parsed[SEG]);
         }
      }

      if (parsed[LABEL])
      {
         smb_t smb;
         smb.nombre = parsed[LABEL];
         smb.val = (char *)malloc(sizeof(char) * 6);
         itoa(nro_instruccion, smb.val, 10);
         agregar_simbolo(simbolos, smb);
      }

      if (parsed[MNEMO])
      {
         nro_instruccion++;
      }
   }
   return nro_instruccion;
}

bool traducir(FILE *archivo, smb_list_t simbolos, int *instrucciones, bool out)
{
   bool ok = TRUE;
   error_t err;

   int instruccion_actual;
   char linea[MAX_LINE_LEN];
   int nro_instruccion = 0;

   while (fgets(linea, MAX_LINE_LEN, archivo))
   {
      char **parsed = parse_line(linea);
      if (parsed[MNEMO])
      {
         err = traducir_instruccion(parsed[MNEMO], parsed[OP1], parsed[OP2], simbolos, &instruccion_actual);
         if (err)
         {
            ok = FALSE;
            instruccion_actual = 0;
            print_err_msg(err);
         }
         if (out)
         {
            printf("%08X  %s", instruccion_actual, linea);
         }
         instrucciones[nro_instruccion++] = instruccion_actual;
      }
      else if (parsed[COMENT] && out) //si hay solo comentario tmb tiene q escribir
      {
        printf("%s", linea);
      }
   }

   return ok;
}

int resolver_simbolos(smb_list_t simbolos, int cant_instrs, int *instrucciones)
{
   int pos_mem = cant_instrs; // empezar al final del codigo.
   node_t *aux = simbolos;
   while (aux)
   {
      if (aux->smb.val[0] == '"')
      {
         // es constante string
         int len = strlen(aux->smb.val);
         // guardar en memoria cada caracter de la string.
         // empieza en 1 y termina en len-1 para eliminar "".
         for (int i = 1; i < len - 1; i++)
         {
            instrucciones[pos_mem + i - 1] = aux->smb.val[i];
         }
         instrucciones[pos_mem + len - 2] = '\0';
         // guardar como valor del simbolo la posicion de memoria donde fue guardada la string.
         itoa(pos_mem, aux->smb.val, 10);
         // aumentar la posicion de memoria
         pos_mem += len - 1;
      }
      aux = aux->sig;
   }
   return pos_mem;
}

void debug_simbolos(smb_list_t simbolos, segmentos_t segmentos, int *instrucciones, int nro_instrs)
{
   printf("Programa:\n");
   printf("Cantidad de instrucciones: %d\n", nro_instrs);
   printf("Tama�o de los segmentos: \n");
   printf(" Code: %d \n", segmentos.cs);
   printf(" Data: %d \n", segmentos.ds);
   printf(" Extra: %d\n", segmentos.es);
   printf(" Stack: %d\n", segmentos.ss);
   printf("Simbolos: \n");
   node_t *aux = simbolos;
   while (aux)
   {
      int val;
      valor_simbolo(aux->smb, &val);
      printf(" Simbolo:\n");
      printf("   Nombre: %s\n", aux->smb.nombre);
      printf("   Val: %d\n", val);
      aux = aux->sig;
   }
   printf("Memoria luego de instrucciones: (instrucciones[i] con i >= nro_instrs)\n");
   int i = nro_instrs;
   while (i < segmentos.cs)
   {
      printf("[%04d] ", i);
      if (instrucciones[i] == '\0')
      {
         printf("str_end\n");
      }
      else
      {
         printf("%c\n", instrucciones[i]);
      }
      i++;
   }
}



