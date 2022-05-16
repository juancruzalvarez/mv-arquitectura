#ifndef ERROR_H
#define ERROR_H

#define IF_ERR_RETURN(exp) \
   do                      \
   {                       \
      error_t e = exp;     \
      if (e)               \
         return e;         \
   } while (0)
   
typedef enum error_t
{
   NO_ERR = 0,
   ERR_NUM_INVALIDO,
   ERR_REG_INVALIDO,
   ERR_MNEM_DESCONOCIDO,
   ERR_OPERADOR_NULO,
   ERR_SIMBOLO_DESCONOCIDO,
   ERR_SIMBOLO_DUPLICADO,
   ERR_SIMBOLO_VALOR_INVALIDO,
   ERR_CORCHETE_SIN_CERRAR
} error_t;

void print_err_msg(error_t e);

#endif