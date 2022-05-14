#include "consts.h"
#include <string.h>
#define CANT_MNEMOS 32
#define CANT_REG 10

typedef struct nom_val{
   char* nom;
   int val;
}nom_val;

const nom_val mnemos[] = {
   {"MOV", 0}, {"ADD", 1}, {"SUB", 2}, {"SWAP", 3},
   {"MUL", 4}, {"DIV", 5}, {"CMP", 6}, {"SHL", 7},
   {"SHR", 8}, {"AND", 9}, {"OR", 0xA}, {"XOR", 0xB},
   {"SLEN", 0xC},{"SMOV", 0xD},{"SCMP", 0xE},{"SYS", 0xF0},
   {"JMP", 0xF1}, {"JZ", 0xF2}, {"JP", 0xF3},{"JN", 0xF4},
   {"JNZ", 0xF5}, {"JNP", 0xF6}, {"JNN", 0xF7},{"LDL", 0xF8},
   {"LDH", 0xF9}, {"RND", 0xFA}, {"NOT", 0xFB},{"PUSH", 0xFC},
   {"POP", 0xFD}, {"CALL", 0xFE},{"RET",0xFF0},{"STOP", 0xFF1}
};
const nom_val registros[] = {
   {"DS",0},
   {"SS",1},
   {"ES",2},
   {"CS",3},
   {"HP",4},
   {"IP",5},
   {"SP",6},
   {"BP",7},
   {"CC",8},
   {"AC",9}
};

int get_mnemo_val(char *mnemo)
{  
   for(int i = 0; i<CANT_MNEMOS;i++){
      if(strcmp(mnemo, mnemos[i].nom) == 0)
         return mnemos[i].val;
   }
   return -1;
}

int get_reg_val(char *reg)
{
   for(int i = 0; i<CANT_REG;i++){
      if(strcmp(reg, registros[i].nom) == 0)
         return registros[i].val;
   }
   return -1;
}