#include "error.h"
#include <stdio.h>
void print_err_msg(error_t e)
{
   switch (e)
   {
   case ERR_NUM_INVALIDO:
      printf("Constante numerica invalida.");
      break;
   case ERR_REG_INVALIDO:
      printf("Registro invalido.");
      break;
   case ERR_MNEM_DESCONOCIDO:
      printf("Nnemonico desconocido.");
      break;
   case ERR_SIMBOLO_DESCONOCIDO:
      printf("Simbolo desconocido.");
      break;
   case ERR_SIMBOLO_DUPLICADO:
      printf("Simbolo duplicado");
      break;
   case ERR_CORCHETE_SIN_CERRAR:
      printf("Se esperaba ']'");
      break;
   case ERR_SIMBOLO_VALOR_INVALIDO:
      printf("El valor del simbolo es invalido.");
      break;
   default:
      printf("Error desconocido.");
   }
}
