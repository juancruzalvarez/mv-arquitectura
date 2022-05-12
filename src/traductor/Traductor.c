#include "traductor.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "consts.h"


void traducir_instruccion(char *mnemo, char *op1, char *op2, int *val);
{
   
}

void traducir_mnemo(char *mnemo, int *val, TIPO_INSTRUCCION *tipo);
{

}

void traducir_operando(char *op, int *val, TIPO_OPERANDO *tipo, smb_list simbolos)
{
   char pri = op[0];
   if (pri == '[')
   {
      op++; // sacar '['
      if (es_letra(op[0]))
      {
         // operador indirecto
         // intenta divir la cadena en el +/-
         int reg, offset = 0;
         char *reg_cad = strtok(op, "+-");
         reg = reg_to_int(reg_cad);
         char *offset_cad = strtok(NULL, "+-");
         // si se pudo dividir leer el offset
         if (offset_cad)
         {
            if (es_letra(offset_cad[0]))
            {
               // offset es un simbolo
               simbolo smb;
               if (!buscar_simbolo(simbolos, offset_cad, &smb))
               {
                  // simbolo inexistente
               }
               offset = smb.val;
            }
            else
            {
               // offset es un numero
               offset = str_to_int(offset_cad);
            }
         }

         *val = (offset << 4) | reg;
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
      simbolo smb;
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

bool es_digito(char c)
{
   return c >= '0' && c <= '9';
}

bool es_letra(char c)
{
   return c >= 'A' && c <= 'Z';
}

bool buscar_simbolo(smb_list simbolos, char *nombre, simbolo *smb)
{
   node *aux = simbolos;
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

   // verificar errores strtol?
   return strtol(cad, NULL, base);
}

int reg_to_int(char *reg)
{
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
      }
      return (reg[0] - 55) | pre;
   }
   }
   return 0;
}

void agregar_simbolo(smb_list *simbolos, simbolo smb)
{
   node *nuevo = (node *)malloc(sizeof(node));
   nuevo->smb = smb;
   nuevo->sig = *simbolos;
   *simbolos = nuevo;
}
