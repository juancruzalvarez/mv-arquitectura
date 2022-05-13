#include "traductor.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "consts.h"


int traducir_instruccion(char *mnemo, char *op1, char *op2, smb_list_t simbolos)
{
   TIPO_INSTRUCCION tipo;
   int val_mnemo;
   traducir_mnemo(mnemo, &val_mnemo, &tipo);
   switch (tipo)
   {
   case DOS_OP:{
      //verificar truncamiento y ver signo?
      TIPO_OPERANDO tipo_op1, tipo_op2;
      int val_op1, val_op2;
      traducir_operando(op1, &val_op1, &tipo_op1, simbolos);
      traducir_operando(op2, &val_op2, &tipo_op2, simbolos);
      return (val_mnemo<<28)|(tipo_op1<<26)|(tipo_op2<<24)|(val_op1<<12)|val_op2;
   }
   case UN_OP:{
      TIPO_OPERANDO tipo_op;
      int val_op;
      traducir_operando(op1, &val_op, &tipo_op, simbolos);
      return (15<<28)|(val_mnemo<<24)|(tipo_op<<22)|val_op;
   }
   case NO_OP:{
      return (255<<24)|(val_mnemo<<20);
   }
   }
}

void traducir_mnemo(char *mnemo, int *val, TIPO_INSTRUCCION *tipo)
{
   *val = get_mnemo_val(mnemo);
   if(*val == -1)
      err = MNEM_DESCONOCIDO;
   
   if(*val < 16){
      *tipo = DOS_OP;
   }else if(*val < 254){
      *tipo = UN_OP;
   }else{
      *tipo = NO_OP;
   }
}

void traducir_operando(char *op, int *val, TIPO_OPERANDO *tipo, smb_list_t simbolos)
{
   char pri = op[0];
   if (pri == '[')
   {
      op++; // sacar '['
      if (es_letra(op[0]))
      {
         // operador indirecto
         // intenta divir la cadena en el +/-
         int reg_val, offset_val = 0;
         char *reg = strtok(op, "+-");
         reg_val = reg_to_int(reg);
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
                  err = SIMBOLO_DESCONOCIDO;
               }
               offset_val = smb.val;
            }
            else
            {
               // offset es un numero
               offset_val = str_to_int(offset);
            }
         }

         *val = (offset_val << 4) | reg_val;
         *tipo = INDIRECTO;
      }
      else
      {
         // operador directo
         *val = str_to_int(op);
         *tipo = DIRECTO;
      }
   }
   else if (es_letra(pri))
   {
      // reg o simbolo
      smb_t smb;
      if (buscar_simbolo(simbolos, op, &smb))
      {
         *val = smb.val;
         *tipo = INMEDIATO;
      }
      else
      {
         *val = reg_to_int(op);
         *tipo = REGISTRO;
      }
   }
   else
   {
      *val = str_to_int(op);
      *tipo = INMEDIATO;
   }
}

int str_to_int(char *cad)
{
   int base = 10;
   switch (cad[0])
   {
   case '\'':
      // lee un caracter
      return cad[1];
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

   
   char* desp;
   errno = 0;
   int val = strtol(cad, &desp, base);

   // verificar errores strtol
   if(cad == desp || errno || *desp != '\0')
      err = NUM_INVALIDO;
   
   return val;
}

int reg_to_int(char *reg)
{
   int aux = get_reg_val(reg);
   if(aux != -1){
      return aux;
   }

   switch (strlen(reg))
   {
   case 3:
      // registro completo  EAX, EBX, ECX, -> 0x0A, 0x0B, 0x0C ...
      // convierte el valor del caracter del medio ( eAx, eBx) a su valor en decimal: 'a'-55 = 10 = 0x0A, 'b'-55 = 11 = 0x0B ...
      return (reg[1] - 55);
   case 2:
   {
      // registro parcial AX, AL, BH
      // Se fija el ultimo caracter para determinar el byte mas significativo
      //  X-> 0x30, H -> 0x20, L-> 0x10
      // luego convierte el primer caracter de la misma manera que en el caso del registro completo y combina los numeros.
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
         err = REG_INVALIDO;
      }
      return (reg[0] - 55) | pre;
   }
   default:
      err = REG_INVALIDO;
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
