#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "error.h"
#include "traductor.h"
#include "parser.h"

#define MAX_LINE_LEN 256
#define MAX_PROG_LEN 256

/*
 TODO:
 argumentos
 escribir binario
 leer correctamente constantes
 hacer que parser lea todo en mayusc
*/

const char* path = "test.txt";

//lee el archivo y guarda en simbolos todos los simbolos encontrados.
void leer_simbolos(FILE* archivo, smb_list_t* simbolos);

//lee el archivo y traduce todas las instrucciones encontradas.
//dependiendo de la variable out, escribe o no a consola la traduccion.
//devuelve TRUE si no hubo errores, sino false.
bool traducir(FILE* archivo, smb_list_t simbolos, int *instrucciones, bool out);

int main()
{
   FILE* archivo_asm = fopen(path, "rt");
   smb_list_t simbolos = NULL;
   int instrucciones[MAX_PROG_LEN];
   bool write = TRUE, out = TRUE;

   if(!archivo_asm){
      printf("No se pudo abrir el archivo asm.\n");
      exit(-1);
   }

   leer_simbolos(archivo_asm, &simbolos);
   rewind(archivo_asm);
   write = traducir(archivo_asm, simbolos, instrucciones, out);
   fclose(archivo_asm);
   if(write){
      //escribir binario
   }
}

void leer_simbolos(FILE* archivo, smb_list_t* simbolos){
   // TODO: leer y guardar los tamaños de los segmentos.
   char linea[MAX_LINE_LEN]; 
   int nro_instruccion = 0;
   while(fgets(linea, MAX_LINE_LEN, archivo)) {
      char** parsed = parse_line(linea);
      if(parsed[CONST] && parsed[CONST_VAL]){
         //TODO: leer bien las constantes.
         agregar_simbolo(simbolos, (smb_t){parsed[CONST], parsed[CONST_VAL]});
      }
      if(parsed[LABEL]){
         agregar_simbolo(simbolos, (smb_t){parsed[LABEL], nro_instruccion});
      }
      if(parsed[MNEMO]){
         nro_instruccion++;
      }
   }
}


bool traducir(FILE* archivo, smb_list_t simbolos, int *instrucciones, bool out){
   bool ok = TRUE;
   error_t err;

   int instruccion_actual;
   char linea[MAX_LINE_LEN]; 
   int nro_instruccion = 0;

   while(fgets(linea, MAX_LINE_LEN, archivo)) {
      char** parsed = parse_line(linea);
      if(parsed[MNEMO]){
         err = traducir_instruccion(parsed[MNEMO], parsed[OP1], parsed[OP2], simbolos, &instruccion_actual);
         if(err){
            ok = FALSE;
            instruccion_actual = 0;
         }
         if(out){
            printf("%08X    %s",instruccion_actual, linea);
            if(err) print_err_msg(err);
         }
         instrucciones[nro_instruccion++] = instruccion_actual;
      }
   }

   return !ok;
}
