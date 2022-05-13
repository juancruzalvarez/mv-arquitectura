#include "error.h"
#include <stdio.h>
void print_err_msg(error_t e){
   switch (e)
   {
   case NUM_INVALIDO:
      printf("Constante numerica invalida.");
      break;
   case REG_INVALIDO:
      printf("Registro invalido.");
      break;
   case MNEM_DESCONOCIDO:
      printf("Nnemonico desconocido.");
      break;
   case SIMBOLO_DESCONOCIDO:
      printf("Simbolo desconocido.");
      break;
   case SIMBOLO_DUPLICADO:
      printf( "Simbolo duplicado");
      break;
   case CORCHETE_SIN_CERRAR:
     printf("Se esperaba ']'");
     break;
   case SIMBOLO_VALOR_INVALIDO:
     printf("El valor del simbolo es invalido.");
     break;
   default:
     printf("Error desconocido.");
   }
}
