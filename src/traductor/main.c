#include <stdio.h>
#include <stdint.h>
#include "traductor.h"
#include "consts.h"

void print_binary(uint32_t x);

int main()
{
   smb_list_t simbolos;
   agregar_simbolo(&simbolos, (smb_t){"nom1", 55});
   agregar_simbolo(&simbolos, (smb_t){"nom4", 21});
   agregar_simbolo(&simbolos, (smb_t){"nom3", 25});
   agregar_simbolo(&simbolos, (smb_t){"nom8", 100});
   smb_t aux;
   print_binary(traducir_instruccion("MOV", "EAX", "65", simbolos));
   printf("  MOV EAX, 65");
}

void print_binary(unsigned int number)
{
    if (number >> 1) {
        print_binary(number >> 1);
    }
    putc((number & 1) ? '1' : '0', stdout);
}