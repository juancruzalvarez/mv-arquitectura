#include "traductor.h"
#include "consts.h"

int main()
{
   smb_list simbolos;
   agregar_simbolo(&simbolos, (simbolo){"nom1", 55});
   agregar_simbolo(&simbolos, (simbolo){"nom4", 21});
   agregar_simbolo(&simbolos, (simbolo){"nom3", 25});
   agregar_simbolo(&simbolos, (simbolo){"nom8", 100});
   simbolo aux;
   printf("nom1 es simbolo? %d\n", buscar_simbolo(simbolos, "nom1", &aux));
   printf("nom4 es simbolo? %d\n", buscar_simbolo(simbolos, "nom4", &aux));
   printf("nom es simbolo? %d\n", buscar_simbolo(simbolos, "nom", &aux));

   printf("val de nmemo (MOV):%x \n", get_mnemo_val("MOV"));
   printf("val de nmemo (JZ):%x \n", get_mnemo_val("JZ"));
   printf("val de nmemo (SCMP):%x \n", get_mnemo_val("SCMP"));
   printf("val de nmemo (AA):%x \n", get_mnemo_val("AA"));
   printf("val de nmemo ():%x \n", get_mnemo_val(""));

   printf("val de reg (CC):%x \n", get_reg_val("CC"));
   printf("val de reg (IP):%x \n", get_reg_val("IP"));
   printf("val de reg (HP):%x \n", get_reg_val("HP"));
   printf("val de reg (DS):%x \n", get_reg_val("DS"));
   printf("val de reg (AA):%x \n", get_reg_val("AA"));
   printf("val de reg ():%x \n", get_reg_val(""));

   printf("EAX: %X \n", reg_to_int("EAX"));
   printf("AH: %X\n", reg_to_int("AH"));
   printf("ECX: %X\n", reg_to_int("ECX"));
   printf("CL: %X\n", reg_to_int("CL"));
   printf("DX: %X\n", reg_to_int("DX"));
   printf("Read num(576): %d\n", str_to_int("576"));
   printf("Read num(@576): %d\n", str_to_int("@576"));
   printf("Read num(#576): %d\n", str_to_int("#576"));
   printf("Read num(%32): %d\n", str_to_int("%32"));
   printf("Read num('a'): %d\n", str_to_int("'a'"));
   printf("Read num('A'): %d\n", str_to_int("'A'"));
}