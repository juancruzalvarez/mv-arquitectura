#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "error.h"
#include "traductor.h"
#include "parser.h"

#define EXTENSION_ASM ".asm"     // extension del archivo que contiene el codigo asm.
#define EXTENSION_BINARIO ".mv2" // extension del archivo binario generado.
#define MAX_CELDAS_MEM 8192      // maximas celdas de memoria de un programa.
#define DEFAULT_SEG_SIZE 1024    // tamaño por defecto de cada uno de los segmentos(menos cs).
#define MAX_LINE_LEN 10000       // cantidad maxima de caracteres por linea en el codigo asm.     ??
#define MAX_PROG_LEN 10000       // cantidad maxima de instrucciones que puede tener un programa. ??

/*
 TODO:
 argumentos (dejar para el final, asi es mas facil de probar)
 escribir binario
 hacer que parser lea todo en mayusc
usar free_line del parser luego de leer??
verificar los tamaños de los segmentos
*/

typedef struct segmentos_t
{
   int cs, // tamaño code seg
       ds, // tamaño data seg
       es, // tamaño extra seg
       ss; // tamaño stack seg
} segmentos_t;

const char *path = "test.txt";

// lee los simbolos(constants, rotulos) y lee las directivas de tamaño de segmento.
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
// muestra tamaño de los segmentos, lista de simbolos con sus respectivos valores,
// cantidad de instrucciones del programa, y la memoria destinada a almacenar las constantes string.
void debug_simbolos(smb_list_t simbolos, segmentos_t seg, int *instrucciones, int nro_instrs);

int main()
{

   FILE *archivo_asm = fopen(path, "rt");
   smb_list_t simbolos = NULL;
   segmentos_t segmentos = {DEFAULT_SEG_SIZE, DEFAULT_SEG_SIZE, DEFAULT_SEG_SIZE, DEFAULT_SEG_SIZE};
   int instrucciones[MAX_PROG_LEN];
   int nro_instrs = -1;
   bool write = TRUE, out = TRUE;

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

         if (strcmp(parsed[SEG], "DATA") == 0)
         {
            segmentos->ds = atoi(parsed[SEG_SIZE]);
         }
         else if (strcmp(parsed[SEG], "EXTRA") == 0)
         {
            segmentos->es = atoi(parsed[SEG_SIZE]);
         }
         else if (strcmp(parsed[SEG], "STACK") == 0)
         {
            segmentos->ss = atoi(parsed[SEG_SIZE]);
         }
         else
         {
            // err ?
         }
      }

      if (parsed[LABEL])
      {

         smb_t smb;
         smb.nombre = parsed[LABEL];
         // TODO: verificar esto.
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
   printf("Tamaño de los segmentos: \n");
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
