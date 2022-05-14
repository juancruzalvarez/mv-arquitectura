#include "error.h"
#include <stdio.h>
void print_err_msg(error_t e)
{
   switch (e)
   {
   case ERR_NUM_INVALIDO:
      printf("ERROR Constante numerica invalida. \n");
      break;
   case ERR_REG_INVALIDO:
      printf("ERROR Registro invalido. \n");
      break;
   case ERR_MNEM_DESCONOCIDO:
      printf("ERROR Nnemonico desconocido. \n");
      break;
   case ERR_SIMBOLO_DESCONOCIDO:
      printf("ERROR Simbolo desconocido. \n");
      break;
   case ERR_SIMBOLO_DUPLICADO:
      printf("ERROR Simbolo duplicado \n");
      break;
   case ERR_CORCHETE_SIN_CERRAR:
      printf("ERROR Se esperaba ']' \n");
      break;
   case ERR_SIMBOLO_VALOR_INVALIDO:
      printf("ERROR El valor del simbolo es invalido. \n");
      break;
   default:
      printf("ERROR Error desconocido. \n");
   }
}
