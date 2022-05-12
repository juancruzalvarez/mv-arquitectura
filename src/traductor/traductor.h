#define TRUE 1
#define FALSE 0
typedef uint8_t bool;

enum ERR
{
   NO_ERR = 0,
   OP_INVALIDO
};

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
} simbolo;

// tipos lista de simbolos.
typedef struct node
{
   simbolo smb;
   struct node *sig;
} node;
typedef node *smb_list;

// dado cadenas representando un mnemonico, y los dos operadores posibles, traduce la instruccion a codigo de maquina
// y lo devuelve en *val.
void traducir_instruccion(char *mnemo, char *op1, char *op2, int *val);

// dado un cadena que contiene un mnemonico, devuelve el valor numerico de esta y su tipo correspondiente.
void traducir_mnemo(char *mnemo, int *val, TIPO_INSTRUCCION *tipo);

// dado una cadena que representa un operando(ej: "24", "[EDX+2]", "%2"), devuelve el valor y el tipo.
void traducir_operando(char *op, int *val, TIPO_OPERANDO *tipo, smb_list simbolos);

// dado una cadena que representa un registro(ej: "EAX", "AH", "CC") devuelve el valor.
int reg_to_int(char *reg);

// dado una cadena que representa un valor numerico(ej: "%32", "#45", "-315"), devuelve un int con el valor.
int str_to_int(char *cad);

// agrega un simbolo a la lista de simbolos.
void agregar_simbolo(smb_list *l, simbolo s);

// verifica si existe un simbolo con ese nombre, y si esta lo devuelve en *smb.
bool buscar_simbolo(smb_list simbolos, char *nombre, simbolo *smb);

// verifica si un caracter es un digito decimal(0-9).
bool es_digito(char c);

// verifica si un caracter es una letra. Solo verifica si es una letra mayuscula, ya que todos los valores se cambian a mayuscula.
bool es_letra(char c);