#ifndef TRADUCTOR_H
#define TRADUCTOR_H

#include <stdint.h>
#include "error.h"

#define TRUE 1
#define FALSE 0
typedef uint8_t bool;

// tipos de operando.
typedef enum TIPO_OPERANDO
{
   INMEDIATO = 0, // operando inmediato   ej: 57, #874, simbolo
   REGISTRO,      // operando de registro ej: EAX, DH, AC
   DIRECTO,       // operando directo     ej: [24], [%32], [0]
   INDIRECTO      // operando indirecto   ej: [CX], [AC+1], [EDX+simbolo]
} TIPO_OPERANDO;

// tipos de instruccion.
typedef enum TIPO_INSTRUCCION
{
   DOS_OP, // dos operadores
   UN_OP,  // un operador
   NO_OP   // ningun operador
} TIPO_INSTRUCCION;

// los simbolos son constantes o rotulos.
typedef struct
{
   char *nombre;
   int val;
} smb_t;

// tipos lista de simbolos.
typedef struct node_t
{
   smb_t smb;
   struct node_t *sig;
} node_t;
typedef node_t* smb_list_t;

// dado cadenas representando un mnemonico, y los dos operadores posibles, traduce la instruccion a codigo de maquina
// y lo devuelve en *val.
error_t traducir_instruccion(char *mnemo, char *op1, char *op2, smb_list_t simbolos, int *val);

// dado un cadena que contiene un mnemonico, devuelve el valor numerico de esta y su tipo correspondiente.
error_t traducir_mnemo(char *mnemo, int *val, TIPO_INSTRUCCION *tipo);

// dado una cadena que representa un operando(ej: "24", "[EDX+2]", "%2"), devuelve el valor y el tipo.
error_t traducir_operando(char *op, int *val, TIPO_OPERANDO *tipo, smb_list_t simbolos);

// dado una cadena que representa un registro(ej: "EAX", "AH", "CC") devuelve el valor.
error_t reg_to_int(char *reg, int *val);

// dado una cadena que representa un valor numerico(ej: "%32", "#45", "-315"), devuelve un int con el valor.
error_t str_to_int(char *cad, int *val);

// agrega un simbolo a la lista de simbolos.
void agregar_simbolo(smb_list_t *l, smb_t smb);

// verifica si existe un simbolo con ese nombre, y si esta lo devuelve en *smb.
bool buscar_simbolo(smb_list_t simbolos, char *nombre, smb_t *smb);

// verifica si un caracter es un digito decimal(0-9).
bool es_digito(char c);

// verifica si un caracter es una letra. Solo verifica si es una letra mayuscula, ya que todos los valores se cambian a mayuscula.
bool es_letra(char c);

#endif