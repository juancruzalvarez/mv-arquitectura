#ifndef ERROR_H
#define ERROR_H

typedef enum error_t
{
   NO_ERR = 0,
   NUM_INVALIDO,
   REG_INVALIDO,
   MNEM_DESCONOCIDO,
   SIMBOLO_DESCONOCIDO,
   SIMBOLO_DUPLICADO,
   CORCHETE_SIN_CERRAR
}error_t;

void print_err_msg(error_t e);

#endif