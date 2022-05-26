#ifndef INSTRUCCIONES_H_INCLUDED
#define INSTRUCCIONES_H_INCLUDED

#include "MaquinaVirtual.h"

void MOV  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void ADD  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void SUB  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void SWAP (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void MUL  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void DIV  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void CMP  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void SHL  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void SHR  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void AND  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void OR   (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void XOR  (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void SLEN (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void SMOV (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void SCMP (MV* maquina, int op1, int op2, int tipo1, int tipo2);
void SYS  (MV* maquina, int op1, int tipo1);
void JMP  (MV* maquina, int op1, int tipo1);
void JZ   (MV* maquina, int op1, int tipo1);
void JP   (MV* maquina, int op1, int tipo1);
void JN   (MV* maquina, int op1, int tipo1);
void JNZ  (MV* maquina, int op1, int tipo1);
void JNP  (MV* maquina, int op1, int tipo1);
void JNN  (MV* maquina, int op1, int tipo1);
void LDL  (MV* maquina, int op1, int tipo1);
void LDH  (MV* maquina, int op1, int tipo1);
void RND  (MV* maquina, int op1, int tipo1);
void NOT  (MV* maquina, int op1, int tipo1);
void PUSH (MV* maquina, int op1, int tipo1);
void POP  (MV* maquina, int op1, int tipo1);
void CALL (MV* maquina, int op1, int tipo1);
void RET  (MV* maquina);
void STOP (MV* maquina);

void Breakpoint(MV* maquina);

int posicionVDD(int,int,int,VDD);
void lecturaVDD(VDD disco,int c,int h,int s,int buffer,int cant ,MV* maquina);
void escrituraVDD(VDD disco,int c, int h, int s, int buffer, int cant,MV* maquina);

typedef void (*F0)(MV*);                       //instruccion 0 operadores
typedef void (*F1)(MV*, int, int);             //instruccion 1 operador
typedef void (*F2)(MV*, int, int, int ,int);   //instruccion 2 operadores

extern F0 Instrucciones_0 [2];
extern F1 Instrucciones_1 [15];
extern F2 Instrucciones_2 [15];

#endif // INSTRUCCIONES_H_INCLUDED
