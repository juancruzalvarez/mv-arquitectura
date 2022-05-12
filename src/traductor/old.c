/*TRADUCTOR MAQUINA VIRTUAL*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define FALSE 0
#define TRUE 1
#include "parser.h"

typedef struct
{
   char rotulo[10];
   int nroinst;
} rots;
rots rotulos[50]; // global x ahora para buscar el rotulo, sin pasar como argumento rotulos en funcion.

int o = FALSE;     // indica si escribe en pantalla
int error = FALSE; // Si hay error no escribe el binario

int main(int argc, char *argv[])
{

   int instrucciones[100];
   char nombreASM[100];
   char nombreMV1[100];

   for (int i = 0; i < argc; i++)
   {
      printf("%d %s \n", i, argv[i]);
      if (strstr(argv[i], ".asm") != 0)
         strcpy(nombreASM, argv[i]);
      else if (strstr(argv[i], ".mv1") != 0)
         strcpy(nombreMV1, argv[i]);
      else if (strstr(argv[i], "-o") != 0)
         o = TRUE;
   }

   char line[1024];
   int i, r = 0;
   int nroinst = 0;
   FILE *arch;

   arch = fopen(nombreASM, "r"); // primera lectura guarda todos los rotulos.
   if (arch == NULL)
   {
      printf("No se encontro archivo");
      exit(1);
   }

   while (fgets(line, 1024, arch) != NULL /*&& (line[0] != '\n')*/)
   {
      if (line[0] != '\n')
      {
         char **parsed = parseline(line);
         if (parsed[1] != NULL)
         {
            if (parsed[0] != NULL) // Si tiene rotulo, guardo nro instruccion de esa linea.
            {
               Mayus(parsed[0]); // pasa a mayus el rotulo
               rotulos[r].nroinst = nroinst;
               strcpy(rotulos[r].rotulo, parsed[0]);
               r++;
            }
            freeline(parsed);
            nroinst++;
         }
      }
   }
   fclose(arch);

   nroinst = 0;
   arch = fopen(nombreASM, "r"); // 2da lectura lee y escibr en pantalla.
   if (arch == NULL)
      exit(1);
   while (fgets(line, 1024, arch) != NULL /*&& (line[0] != '\n')*/) // obtiene linea por linea de archivo
   {
      if (line[0] != '\n')
      {
         char **parsed = parseline(line); // graba la linea del .txt

         if (parsed[0] != NULL)
            Mayus(parsed[0]);

         if (parsed[1] != NULL)
            Mayus(parsed[1]);

         if (parsed[2] != NULL)
            Mayus(parsed[2]);

         if (parsed[3] != NULL)
            Mayus(parsed[3]);

         if (parsed[4] != NULL)
            Mayus(parsed[4]);

         int inst = 0;
         int mne = -1, opa = 0, opb = 0;
         int topb = 0, topa = 0;

         if (parsed[1] != NULL)
         {
            StringInstruccion(parsed[1], &mne); // busco instruccion
            if (mne >= 0 && mne <= 12)          // 2 operandos
            {
               StringInt(parsed[2], &opa, &topa);
               StringInt(parsed[3], &opb, &topb);
               if (topa == 0 && opa > 4095) // || topa==0 && opa<-2048) //inmediato, trunca si es mayor a 2047 (12bits)
               {
                  opa = opa & 0xFFF;
                  if (o == TRUE)
                     printf("Advertencia truncado operando inmediato mayor a 12 bits  \n");
               }
               if (topb == 0 && opb > 4095) //|| topa==0 && opa<-2048 ) //lo mismo para 2do operando
               {
                  opb = opb & 0xFFF;
                  if (o == TRUE)
                     printf("Advertencia truncado operando inmediato mayor a 12 bits  \n");
               }

               inst = (mne << 28) | ((topa << 26) & 0x0C000000) | ((topb << 24) & 0x03000000) |
                      ((opa << 12) & 0x00FFF000) | (opb & 0x00000FFF);
               if (o == TRUE)
               {
                  printf("%04d", nroinst);
                  printf(" ");
                  printf("%08X ", inst);
                  printf(" ");
                  for (int n = 0; n < 5; n++)
                     imprime(parsed[n]);
                  printf("\n");
                  // printf("%s %s %s %s %s \n",parsed[0], parsed[1], parsed[2], parsed[3], parsed[4]);
               }
            }
            else if (mne >= 0xF0 && mne <= 0xFB) // 1 operando, de F0 a FB en hex.
            {
               StringInt(parsed[2], &opa, &topa);

               if (topa == 0 && opa > 65535) //|| topa==0 && opa<-32768) //inmediato, trunca si es mayor a 32767 (16bits)
               {
                  opa = opa & 0xFFFF;
                  if (o == TRUE)
                     printf("Advertencia truncado operando inmediato mayor a 16 bits  \n ");
               }
               inst = (mne << 24) | ((topa << 22) & 0x00C00000) | (opa & 0x0000FFFF);
               if (o == TRUE)
               {
                  printf("%04d", nroinst);
                  printf(" ");
                  printf("%08X ", inst);
                  printf(" ");
                  for (int n = 0; n < 5; n++)
                     imprime(parsed[n]);
                  printf("\n");
                  // printf("%s %s %s %s %s \n",parsed[0], parsed[1], parsed[2], parsed[3], parsed[4]);
               }
            }
            else if (mne == 0xFF1) // Para el STOP, instruccion toma 12 bits.
            {
               inst = mne << 20;
               if (o == TRUE)
               {
                  printf("%04d", nroinst);
                  printf(" ");
                  printf("%08X ", inst);
                  printf(" ");
                  for (int n = 0; n < 5; n++)
                     imprime(parsed[n]);
                  printf("\n");
                  // printf("%s %s %s %s %s \n",parsed[0], parsed[1], parsed[2], parsed[3], parsed[4]);
               }
            }
            else if (mne == -1) // no se encontro instruccion, ERROR TIPO 2.
            {
               if (o == TRUE)
               {
                  printf("%04d", nroinst);
                  printf(" ");
                  printf("%08X ", 0xFFFFFFFF);
                  printf(" ");
                  for (int n = 0; n < 5; n++)
                     imprime(parsed[n]);
                  printf("\n");
                  // printf("%s %s %s %s %s \n",parsed[0], parsed[1], parsed[2], parsed[3], parsed[4]);
               }
            }
            instrucciones[nroinst] = inst;
            nroinst++; // para cuando haya un ciclo de lectura
         }
         else if (parsed[4] != NULL)
            printf("%s \n", parsed[4]);

         freeline(parsed);
      }
   }
   fclose(arch);

   if (error != TRUE) // escritura en binario
   {
      FILE *archb;
      int reserv = 0x0000, k;

      archb = fopen(nombreMV1, "wb+");
      if (archb == NULL)
         exit(1);

      // Header
      fwrite("MV-1", 4, 1, archb);
      fwrite(&nroinst, sizeof(int), 1, archb); // cant instrucciones
      fwrite(&reserv, sizeof(int), 1, archb);  // reservado
      fwrite(&reserv, sizeof(int), 1, archb);
      fwrite(&reserv, sizeof(int), 1, archb);
      fwrite("V.22", 4, 1, archb);

      for (k = 0; k < nroinst; k++)
         fwrite(&instrucciones[k], sizeof(int), 1, archb);

      printf("Se escribio el archivo binario al no existir errores de traduccion \n");

      fclose(archb);
   }

   system("pause");
   return 0;
}

uint16_t reg_to_int(char *reg)
{
   uint16_t val = 0;
   switch (strlen(reg))
   {
   case 3:
      // registro completo  EAX, EBX, ECX, -> 0x0A, 0x0B, 0x0C ...
      // convierte el valor del caracter del medio ( eAx, eBx) a su valor en decimal: 'a'-55 = 10 = 0x0A, 'b'-55 = 11 = 0x0B ...
      return (reg[1] - 55);
   case 2:
      // registro parcial AX, AL, BH
      // Se fija el ultimo caracter para determinar el byte mas significativo
      //  X-> 0x30, H -> 0x20, L-> 0x10
      // luego convierte el primer caracter de la misma manera que en el caso del registro completo y combina los numeros.
      uint8_t pre = 0;
      switch (reg[1])
      {
      case 'x':
         pre = 0x30;
         break;
      case 'h':
         pre = 0x30;
         break;
      case 'l':
         pre = 0x30;
         break;
      default:
         // error?
      }
      return (reg[0] - 55) | pre;
   default:
      // error?
   }
   return 0;
}

// read op.
void StringInt(char *string, int *val, int *tipo)
{

   if (strcmp(string, "EAX") == 0) // Valores para operando registro
   {
      *val = 0x0A;
      *tipo = 1;
   }
   else if (strcmp(string, "AX") == 0)
   {
      *val = 0x3A;
      *tipo = 1;
   }
   else if (strcmp(string, "AH") == 0)
   {
      *val = 0x2A;
      *tipo = 1;
   }
   else if (strcmp(string, "AL") == 0)
   {
      *val = 0x1A;
      *tipo = 1;
   }
   else if (strcmp(string, "EBX") == 0)
   {
      *val = 0x0B;
      *tipo = 1;
   }
   else if (strcmp(string, "BX") == 0)
   {
      *val = 0x3B;
      *tipo = 1;
   }
   else if (strcmp(string, "BH") == 0)
   {
      *val = 0x2B;
      *tipo = 1;
   }
   else if (strcmp(string, "BL") == 0)
   {
      *val = 0x1B;
      *tipo = 1;
   }
   else if (strcmp(string, "ECX") == 0)
   {
      *val = 0x0C;
      *tipo = 1;
   }
   else if (strcmp(string, "CX") == 0)
   {
      *val = 0x3C;
      *tipo = 1;
   }
   else if (strcmp(string, "CH") == 0)
   {
      *val = 0x2C;
      *tipo = 1;
   }
   else if (strcmp(string, "CL") == 0)
   {
      *val = 0x1C;
      *tipo = 1;
   }
   else if (strcmp(string, "EDX") == 0)
   {
      *val = 0x0D;
      *tipo = 1;
   }
   else if (strcmp(string, "DX") == 0)
   {
      *val = 0x3D;
      *tipo = 1;
   }
   else if (strcmp(string, "DH") == 0)
   {
      *val = 0x2D;
      *tipo = 1;
   }
   else if (strcmp(string, "DL") == 0)
   {
      *val = 0x1D;
      *tipo = 1;
   }
   else if (strcmp(string, "EEX") == 0)
   {
      *val = 0x0E;
      *tipo = 1;
   }
   else if (strcmp(string, "EX") == 0)
   {
      *val = 0x3E;
      *tipo = 1;
   }
   else if (strcmp(string, "EH") == 0)
   {
      *val = 0x2E;
      *tipo = 1;
   }
   else if (strcmp(string, "EL") == 0)
   {
      *val = 0x1E;
      *tipo = 1;
   }
   else if (strcmp(string, "EFX") == 0)
   {
      *val = 0x0F;
      *tipo = 1;
   }
   else if (strcmp(string, "FX") == 0)
   {
      *val = 0x3F;
      *tipo = 1;
   }
   else if (strcmp(string, "FH") == 0)
   {
      *val = 0x2F;
      *tipo = 1;
   }
   else if (strcmp(string, "FL") == 0)
   {
      *val = 0x1F;
      *tipo = 1;
   }
   else if (strcmp(string, "DS") == 0)
   {
      *val = 0x0;
      *tipo = 1;
   }
   else if (strcmp(string, "IP") == 0)
   {
      *val = 0x5;
      *tipo = 1;
   }
   else if (strcmp(string, "CC") == 0)
   {
      *val = 0x8;
      *tipo = 1;
   }
   else if (strcmp(string, "AC") == 0)
   {
      *val = 0x9;
      *tipo = 1;
   }
   else if (string[0] == '[') // valor operando directo
   {
      *tipo = 2;
      char *aux = (char *)malloc(strlen(string) + 1); // auxiliar para remover ult caracter que es [
      strcpy(aux, string);
      aux[strlen(aux) - 1] = '\0';
      *aux++;
      if (aux[0] == '%')
      {
         *aux++;
         *val = (int)strtol(aux, NULL, 16);
      }
      else if (aux[0] == '@')
      {
         *aux++;
         *val = (int)strtol(aux, NULL, 8);
      }
      else if (aux[0] == '\'') // revisar
      {
         *val = aux[1];
      }
      else if (aux[0] == '#')
      {
         *aux++;
         *val = (int)strtol(aux, NULL, 10);
      }
      else
         *val = (int)strtol(aux, NULL, 10);
      free(aux);
   }
   else // Valor para operando inmediato
   {
      *tipo = 0;
      if (string[0] == '%')
      {
         *string++;
         *val = (int)strtol(string, NULL, 16);
      }

      else if (string[0] == '@')
      {
         *string++;
         *val = (int)strtol(string, NULL, 8);
      }
      else if (string[0] == '\'') // revisar, acsii
      {
         *val = string[1];
      }
      else if (string[0] == '#')
      {
         *string++;
         *val = (int)strtol(string, NULL, 10);
      }
      else if ((string[0] >= '0' && string[0] <= '9') || string[0] == '-') // valor decimal sin #
         *val = (int)strtol(string, NULL, 10);
      else // es rotulo
      {
         int k = 0;
         while (k < 50 && strcmp(string, rotulos[k].rotulo) != 0) // busco si rotulo es de instruccion anterior.
         {
            k++;
         }
         if (strcmp(string, rotulos[k].rotulo) == 0) // existe rotulo
            *val = rotulos[k].nroinst;
         else
         {
            printf("No existe rotulo indicado, error \n");
            *val = 0xFFF; // tipo de rotulo error. ERROR TIPO 1.
            if (error != TRUE)
               error = TRUE;
         }
      }
   }
}

// read instruccion
void StringInstruccion(char *string, int *val)
{
   if (strcmp(string, "MOV") == 0) // Valores de mnemonicos 2 operandos
   {
      *val = 0;
   }
   else if (strcmp(string, "ADD") == 0)
   {
      *val = 1;
   }
   else if (strcmp(string, "SUB") == 0)
   {
      *val = 2;
   }
   else if (strcmp(string, "SWAP") == 0)
   {
      *val = 3;
   }
   else if (strcmp(string, "MUL") == 0)
   {
      *val = 4;
   }
   else if (strcmp(string, "DIV") == 0)
   {
      *val = 5;
   }
   else if (strcmp(string, "CMP") == 0)
   {
      *val = 6;
   }
   else if (strcmp(string, "SHL") == 0)
   {
      *val = 7;
   }
   else if (strcmp(string, "SHR") == 0)
   {
      *val = 8;
   }
   else if (strcmp(string, "AND") == 0)
   {
      *val = 9;
   }
   else if (strcmp(string, "OR") == 0)
   {
      *val = 0x0A;
   }
   else if (strcmp(string, "XOR") == 0)
   {
      *val = 0x0B;
   }
   else if (strcmp(string, "SYS") == 0) // Valores de mnemonicos 1 operando
   {
      *val = 0xF0;
   }
   else if (strcmp(string, "JMP") == 0)
   {
      *val = 0xF1;
   }
   else if (strcmp(string, "JZ") == 0)
   {
      *val = 0xF2;
   }
   else if (strcmp(string, "JP") == 0)
   {
      *val = 0xF3;
   }
   else if (strcmp(string, "JN") == 0)
   {
      *val = 0xF4;
   }
   else if (strcmp(string, "JNZ") == 0)
   {
      *val = 0xF5;
   }
   else if (strcmp(string, "JNP") == 0)
   {
      *val = 0xF6;
   }
   else if (strcmp(string, "JNN") == 0)
   {
      *val = 0xF7;
   }
   else if (strcmp(string, "LDL") == 0)
   {
      *val = 0xF8;
   }
   else if (strcmp(string, "LDH") == 0)
   {
      *val = 0xF9;
   }
   else if (strcmp(string, "RND") == 0)
   {
      *val = 0xFA;
   }
   else if (strcmp(string, "NOT") == 0)
   {
      *val = 0xFB;
   }
   else if (strcmp(string, "STOP") == 0)
   {
      *val = 0xFF1;
   }
   else
   {
      printf("No se encontro el mnemonico correspondiente, error \n");
      if (error != TRUE)
         error = TRUE;
   }
}

void Mayus(char *cadena)
{
   char *letra = cadena;
   if (letra[0] != '\'')
   {
      while (*letra != '\0')
      {
         if ((int)(*letra) >= 97 && (int)(*letra) <= 122)
            *letra -= ('a' - 'A');
         *letra++;
      }
   }
}

void imprime(char *printer)
{
   if (printer != NULL)
      printf("%s ", printer);
}
