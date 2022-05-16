#include "traductor.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include "consts.h"

error_t traducir_instruccion(char *mnemo, char *op1, char *op2, smb_list_t simbolos, int *val)
{

   TIPO_INSTRUCCION tipo;
   int val_mnemo;
   IF_ERR_RETURN(traducir_mnemo(mnemo, &val_mnemo, &tipo));
   switch (tipo)
   {
   case DOS_OP:
   {
      TIPO_OPERANDO tipo_op1, tipo_op2;
      int val_op1, val_op2;

      IF_ERR_RETURN(traducir_operando(op1, &val_op1, &tipo_op1, simbolos));
      IF_ERR_RETURN(traducir_operando(op2, &val_op2, &tipo_op2, simbolos));

      //*val = (val_mnemo << 28) | (tipo_op1 << 26) | (tipo_op2 << 24) | (val_op1 << 12) | val_op2;
      *val = (val_mnemo << 28) | (tipo_op1 << 26) | (tipo_op2 << 24) |
             (check_trunc(val_op1, 12) << 12)&0xFFF000 | check_trunc(val_op2, 12)&0xFFF;
      break;
   }
   case UN_OP:
   {
      TIPO_OPERANDO tipo_op;
      int val_op;
      IF_ERR_RETURN(traducir_operando(op1, &val_op, &tipo_op, simbolos));
      //*val = (15 << 28) | (val_mnemo << 24) | (tipo_op << 22) | val_op;
      // *val= (15 << 28) | ((tipo_op << 22) & 0x00C00000) | (val_op & 0x0000FFFF);
      *val = (val_mnemo << 24) | (tipo_op << 22) | check_trunc(val_op, 16)&0xFFFF;
      break;
   }
   case NO_OP:
   {
      *val = (255 << 24) | (val_mnemo << 20);
      break;
   }
   }
   return NO_ERR;
}

error_t traducir_mnemo(char *mnemo, int *val, TIPO_INSTRUCCION *tipo)
{
   *val = get_mnemo_val(mnemo);
   if (*val == -1)
      return ERR_MNEM_DESCONOCIDO;

   if (*val < 0xF0)
   {
      *tipo = DOS_OP;
   }
   else if (*val < 0xFF0)
   {
      *tipo = UN_OP;
   }
   else
   {
      *tipo = NO_OP;
   }

   return NO_ERR;
}

error_t traducir_operando(char *op, int *val, TIPO_OPERANDO *tipo, smb_list_t simbolos)
{
   if(!op)
      return ERR_OPERADOR_NULO;
   char pri = op[0];
   if (pri == '[')
   {

      // verificar que cierre corchete.
      int len = strlen(op);
      if (op[len - 1] != ']')
      {
         return ERR_CORCHETE_SIN_CERRAR;
      }
      // eliminar '[' y ']'
      // TODO: deberia verificar q strlen(op) > 2??
      memmove(op, op + 1, len - 2);
      op[len - 2] = '\0';

      if (es_letra(op[0]))
      {
         // operador indirecto
         int reg_val, offset_val = 0;
         int signo;
         if (strchr(op, '+') != NULL)
            signo = 1; // si hay un +
         else
            signo = -1; // si hay un -

         // intenta divir la cadena en el +/-
         char *reg = strtok(op, "+-");
         IF_ERR_RETURN(reg_to_int(reg, &reg_val));
         char *offset = strtok(NULL, "+-");
         // si se pudo dividir leer el offset
         if (offset)
         {
            if (es_letra(offset[0]))
            {
               // offset es un simbolo
               smb_t smb;
               if (!buscar_simbolo(simbolos, offset, &smb))
               {
                  return ERR_SIMBOLO_DESCONOCIDO;
               }
               IF_ERR_RETURN(valor_simbolo(smb, &offset_val));
            }
            else
            {
               // offset es un numero
               IF_ERR_RETURN(str_to_int(offset, &offset_val));
            }
         }
         offset_val = signo * check_trunc( offset_val, 8); // chequeo de truncado por offset de simbolo y aplica el + o -
         *val = (offset_val << 4) | reg_val;
         *tipo = INDIRECTO;
      }
      else
      {
         // operador directo
         IF_ERR_RETURN(str_to_int(op, val));
         *tipo = DIRECTO;
      }
   }
   else if (es_letra(pri))
   {

      // reg o simbolo
      smb_t smb;
      if (buscar_simbolo(simbolos, op, &smb))
      {
         IF_ERR_RETURN(valor_simbolo(smb, val));
         *tipo = INMEDIATO;
      }
      else
      {

         error_t err = reg_to_int(op, val);
         // si hubo error aca probablemente es porque se ingreso un simbolo desconocido,
         // entonces buscar_simbolo devolvio false y quizo leerlo como un registro
         // se devuelve SIMBOLO_DESCONOCIDO
         if (err)
            return ERR_SIMBOLO_DESCONOCIDO;
         *tipo = REGISTRO;
      }
   }
   else
   {
      *tipo = INMEDIATO;
      IF_ERR_RETURN(str_to_int(op, val));
   }
   return NO_ERR;
}

error_t str_to_int(char *cad, int *val)
{
   int base = 10;
   switch (cad[0])
   {
   case '\'':
      // lee un caracter
      *val = cad[1];
      return NO_ERR;
   case '%':
      // lee un numero en hexadecimal, elimina el caracter %
      base = 16;
      cad++;
      break;
   case '#':
      // lee un numero en decimal, elimina el caracter #
      base = 10;
      cad++;
      break;
   case '@':
      // lee un numero en octal, elimina el caracter @
      base = 8;
      cad++;
      break;
   }

   char *desp;
   errno = 0;
   *val = strtol(cad, &desp, base);

   // verificar errores strtol
   if (cad == desp || errno || *desp != '\0')
      return ERR_NUM_INVALIDO;

   return NO_ERR;
}

error_t reg_to_int(char *reg, int *val)
{
   int aux = get_reg_val(reg);
   if (aux != -1)
   {
      *val = aux;
      return NO_ERR;
   }

   switch (strlen(reg))
   {
   case 3:
      // registro completo  EAX, EBX, ECX, -> 0x0A, 0x0B, 0x0C ...
      // convierte el valor del caracter del medio ( eAx, eBx) a su valor en decimal: 'a'-55 = 10 = 0x0A, 'b'-55 = 11 = 0x0B ...
      if (!(reg[1] >= 'A' && reg[1] <= 'F'))
      {
         return ERR_REG_INVALIDO;
      }
      *val = reg[1] - 55;
      return NO_ERR;
   case 2:
   {
      // registro parcial AX, AL, BH
      // Se fija el ultimo caracter para determinar el byte mas significativo
      //  X-> 0x30, H -> 0x20, L-> 0x10
      // luego convierte el primer caracter de la misma manera que en el caso del registro completo y combina los numeros.
      if (!(reg[0] >= 'A' && reg[0] <= 'F'))
      {
         return ERR_REG_INVALIDO;
      }
      uint8_t pre = 0;
      switch (reg[1])
      {
      case 'X':
         pre = 0x30;
         break;
      case 'H':
         pre = 0x20;
         break;
      case 'L':
         pre = 0x10;
         break;
      default:
         return ERR_REG_INVALIDO;
      }
      *val = (reg[0] - 55) | pre;
      return NO_ERR;
   }
   default:
      return ERR_REG_INVALIDO;
   }
}

bool es_digito(char c)
{
   return c >= '0' && c <= '9';
}

bool es_letra(char c)
{
   return c >= 'A' && c <= 'Z';
}

error_t valor_simbolo(smb_t smb, int *val)
{
   if (str_to_int(smb.val, val) != NO_ERR)
      return ERR_SIMBOLO_VALOR_INVALIDO;

   return NO_ERR;
}

bool buscar_simbolo(smb_list_t simbolos, char *nombre, smb_t *smb)
{

   node_t *aux = simbolos;
   while (aux)
   {
      if (strcmp(aux->smb.nombre, nombre) == 0)
      {
         *smb = aux->smb;
         return TRUE;
      }
      aux = aux->sig;
   }
   return FALSE;
}

void agregar_simbolo(smb_list_t *simbolos, smb_t smb)
{
   node_t *nuevo = (node_t *)malloc(sizeof(node_t));
   nuevo->smb = smb;
   nuevo->sig = *simbolos;
   *simbolos = nuevo;
}

int check_trunc(int val, int bits)
{
   int max_val = pow(2, bits)-1;   //maximo valor representable con la cantidad de bits.
   if (val > max_val)
   {
      printf("Warning: El valor %d no puede ser representado por %d bits, se trunca el operando.\n", val, bits);
      return val & max_val;
   }

   return val;
}
