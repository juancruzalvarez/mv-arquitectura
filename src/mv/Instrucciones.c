#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "Instrucciones.h"

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define ES_DIGITO(a) ((a >= 48) && (a <= 57))

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define ClearScreen() system("clear");
#endif

#if defined(_WIN32) || defined(_WIN64)
#define ClearScreen() system("cls");
#endif

void MostrarRangoMemoria(MV maquina, int mem_start, int mem_end)
{
    for (int i = MAX(0, mem_start); i <= MIN(CELDAS_MEMORIA - 1, mem_end); i++)
    {
        printf("%s[%d] %x\n", i == maquina.registros[IP] ? ">" : " ", i, maquina.memoria[i]);
    }
}

int EncontrarChar(char *str, int str_len, char c)
{
    for (int i = 0; i < str_len; i++)
    {
        if (str[i] == c)
            return i;
    }
    return -1;
}

int GetNum(char *str, int str_len)
{
    int n = 0;
    for (int i = str_len - 1; i >= 0; i--)
    {
        if (!ES_DIGITO(str[i]))
        {
            return -1;
        }
        n += (str[i] - 48) * pow(10, str_len - 1 - i);
    }
    return n;
}

void Disassemble(MV maquina)
{
    MostrarRangoMemoria(maquina, maquina.registros[IP] - 5, MIN(maquina.registros[IP] + 5, maquina.registros[DS]));
    printf("DS:  %d\n", maquina.registros[DS]);
    printf("IP:  %d\n", maquina.registros[IP]);
    printf("CC: #%x\n", maquina.registros[CC]);
    printf("AC: #%x\n", maquina.registros[AC]);
    printf("A:  #%x\n", maquina.registros[A]);
    printf("B:  #%x\n", maquina.registros[B]);
    printf("C:  #%x\n", maquina.registros[C]);
    printf("D:  #%x\n", maquina.registros[D]);
    printf("E:  #%x\n", maquina.registros[E]);
    printf("F:  #%x\n", maquina.registros[F]);
}

typedef enum TIPO_OPERADOR
{
    INMEDIATO = 0,
    REGISTRO,
    MEMORIA,
    INDIRECTO
} TIPO_OPERADOR;

// devuelve la posicion de memoria absoluta a la que apunta un operador indirecto, o -1 si hay un error.
int GetPosMemoriaIndirecto(MV *maquina, int operador_indirecto)
{
    int reg = operador_indirecto & 0xF;
    int offset = (operador_indirecto >> 4) & 0xFF;
    int regH = (maquina->registros[reg] & 0xFFFF0000) >> 16; // codigo de registro
    int regL = (maquina->registros[reg] & 0x0000FFFF);       // desplazamiento dentro del segmento

    int aux = maquina->registros[regH] & 0x0000FFFF; // parte baja del registro (direccion absoluta)
    int memAbs = aux + regL + offset;
    if (memAbs >= (maquina->registros[DS]) & 0xFFFF)
    {
        if (regL <= (maquina->registros[regH] >> 16) & 0xFFFF)
        {
            return memAbs;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

int GetValor(MV *maquina, int op, int tipo)
{
    int res, aux;
    switch (tipo)
    {

    case INMEDIATO:
    {
        res = op;
        break;
    }
    case MEMORIA:
    {
        res = maquina->memoria[maquina->registros[DS] + op];
        break;
    }
    case REGISTRO:
    {
        if (op > 0x30)
        { // X
            aux = maquina->registros[op - 0x30];
            aux = aux & 0xFFFF;
            if (aux >= 0x8000)
            {
                res = ~0xFFFF;
                res = res | (aux & 0xFFFF);
            }
            else
            {
                res = aux;
            }
        }
        else if (op > 0x20)
        { // H
            aux = maquina->registros[op - 0x20];
            aux = (aux & 0xFF00) >> 8;
            if (aux >= 0x80)
            {
                res = ~0xFF;
                res = res | (aux & 0xFF);
            }
            else
            {
                res = aux;
            }
        }
        else if (op > 0x10)
        { // L
            aux = maquina->registros[op - 0x10];
            aux = aux & 0xFF;
            if (aux >= 0x80)
            {
                res = ~0xFF;
                res = res | (aux & 0xFF);
            }
            else
            {
                res = aux;
            }
        }
        else
        { // EX (registro entero)
            res = maquina->registros[op];
        }
        break;
    }
    case INDIRECTO:
    { //[EAX] ---> op=0x010B  EAX= 0x00200B  ES=0xF00A0005
        int reg = op & 0xF;
        int offset = (op >> 4) & 0xFF;
        int regH = (maquina->registros[reg] & 0xFFFF0000) >> 16; // codigo de registro
        int regL = (maquina->registros[reg] & 0x0000FFFF);       // desplazamiento dentro del segmento

        aux = maquina->registros[regH] & 0x0000FFFF; // parte baja del registro (direccion absoluta)
        int memAbs = aux + regL + offset;
        if (memAbs >= (maquina->registros[DS]) & 0xFFFF)
            if (regL <= (maquina->registros[regH] >> 16) & 0xFFFF)
                res = maquina->memoria[memAbs];
            else
            {
                printf("Segmentation Fault\n");
                res = NULL;
            }
        else
        {
            printf("ERROR!\n"); // Corta ejecucion?
            res = NULL;
        }
        break;
    }
    }

    return res;
}

// falta hacer set valor para operador indirecto.
void SetValor(MV *maquina, int op, int tipo, int valor)
{
    switch (tipo)
    {
    case MEMORIA:
    {
        maquina->memoria[maquina->registros[DS] + op] = valor;
        break;
    }
    case REGISTRO:
    {
        if (op > 0x30)
        {
            maquina->registros[op - 0x30] = maquina->registros[op - 0x30] & ~0xFFFF;
            maquina->registros[op - 0x30] = maquina->registros[op - 0x30] | (valor & 0xFFFF);
        }
        else if (op > 0x20)
        {
            valor = valor << 8;
            maquina->registros[op - 0x20] = maquina->registros[op - 0x20] & ~0xFF00;
            maquina->registros[op - 0x20] = maquina->registros[op - 0x20] | (valor & 0xFF00);
        }
        else if (op > 0x10)
        {
            maquina->registros[op - 0x10] = maquina->registros[op - 0x10] & ~0xFF;
            maquina->registros[op - 0x10] = maquina->registros[op - 0x10] | (valor & 0xFF);
        }
        else
        {
            maquina->registros[op] = valor;
        }
        break;
    }
    }
}

void ActualizarCC(MV *maquina, int res)
{
    if (res > 0) // positivo
        maquina->registros[CC] = 0x00000000;
    else if (res < 0) // negativo
        maquina->registros[CC] = 0x80000000;
    else // cero
        maquina->registros[CC] = 0x00000001;
}

int BitN(int cc)
{
    return (cc & 0x80000000);
}

int BitZ(int cc)
{
    return (cc & 0x1);
}

// 2 OPERANDOS
void MOV(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("Move %d %d %d %d\n", op1, op2, tipo1, tipo2);
    SetValor(maquina, op1, tipo1, GetValor(maquina, op2, tipo2));
    maquina->registros[IP]++;
}

void ADD(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("add %d %d %d %d\n ", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) + GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void SUB(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("sub %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) - GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void MUL(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("mul %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) * GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void DIV(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("div %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux, dividendo = GetValor(maquina, op1, tipo1), divisor = GetValor(maquina, op2, tipo2);
    if (divisor != 0)
    { // en caso contrario hace algo?
        aux = dividendo / divisor;
        SetValor(maquina, op1, tipo1, aux);
        SetValor(maquina, AC, REGISTRO, dividendo % divisor); // resto
        ActualizarCC(maquina, aux);
    }
    maquina->registros[IP]++;
}

void SWAP(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("swap %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1);
    SetValor(maquina, op1, tipo1, GetValor(maquina, op2, tipo2));
    SetValor(maquina, op2, tipo2, aux);
    maquina->registros[IP]++;
}

void CMP(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("cmp %d %d %d %d \n", op1, op2, tipo1, tipo2);
    ActualizarCC(maquina, GetValor(maquina, op1, tipo1) - GetValor(maquina, op2, tipo2));
    maquina->registros[IP]++;
}

void AND(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("and %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) & GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void OR(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("or %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) | GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void XOR(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("xor %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) ^ GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void SHL(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    printf("shl %d %d %d %d \n", op1, op2, tipo1, tipo2);
    int aux = GetValor(maquina, op1, tipo1) << GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

// ver lo del signo
void SHR(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    int aux = GetValor(maquina, op1, tipo1) >> GetValor(maquina, op2, tipo2);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void SLEN(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    int pos_memoria;
    if (tipo2 == MEMORIA)
    {
        pos_memoria = maquina->registros[DS] + op2;
    }
    else if (tipo2 == INDIRECTO)
    {
        pos_memoria = GetPosMemoriaIndirecto(maquina, op2);
        if (pos_memoria == -1)
        {
            return;
        }
    }
    else
    {
        // error?
        return;
    }
    int start = pos_memoria;
    while (maquina->memoria[pos_memoria++] != '\0')
        ;
    SetValor(maquina, op1, tipo1, pos_memoria - start);
}

void SMOV(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    int pos_memoria_to;
    int pos_memoria_from;
    if (tipo2 == MEMORIA)
    {
        pos_memoria_to = maquina->registros[DS] + op2;
    }
    else if (tipo2 == INDIRECTO)
    {
        pos_memoria_to = GetPosMemoriaIndirecto(maquina, op2);
        if (pos_memoria_to == -1)
        {
            return;
        }
    }
    else
    {
        // error?
        return;
    }
    if (tipo1 == MEMORIA)
    {
        pos_memoria_from = maquina->registros[DS] + op1;
    }
    else if (tipo1 == INDIRECTO)
    {
        pos_memoria_from = GetPosMemoriaIndirecto(maquina, op1);
        if (pos_memoria_from == -1)
        {
            return;
        }
    }
    else
    {
        // error?
        return;
    }

    while (maquina->memoria[pos_memoria_from] != '\0')
    {
        maquina->memoria[pos_memoria_to++] = maquina->memoria[pos_memoria_from++];
    }
}

void SCMP(MV *maquina, int op1, int op2, int tipo1, int tipo2)
{
    int pos_memoria1;
    int pos_memoria2;
    if (tipo2 == MEMORIA)
    {
        pos_memoria2 = maquina->registros[DS] + op2;
    }
    else if (tipo2 == INDIRECTO)
    {
        pos_memoria2 = GetPosMemoriaIndirecto(maquina, op2);
        if (pos_memoria2 == -1)
        {
            return;
        }
    }
    else
    {
        // error?
        return;
    }
    if (tipo1 == MEMORIA)
    {
        pos_memoria1 = maquina->registros[DS] + op1;
    }
    else if (tipo1 == INDIRECTO)
    {
        pos_memoria1 = GetPosMemoriaIndirecto(maquina, op1);
        if (pos_memoria1 == -1)
        {
            return;
        }
    }
    else
    {
        // error?
        return;
    }
    int val = 0;
    while (
        (val = maquina->memoria[pos_memoria1] - maquina->memoria[pos_memoria2]) == 0
        && maquina->memoria[pos_memoria1++] != '\0'
        && maquina->memoria[pos_memoria2++] != '\0'
    );
    ActualizarCC(maquina, val);

}

// 1 OPERANDO

void JMP(MV *maquina, int op1, int tipo1)
{
    printf("jmp %d %d  \n", op1, tipo1);
    maquina->registros[IP] = GetValor(maquina, op1, tipo1);
}

void JZ(MV *maquina, int op1, int tipo1)
{
    printf("jz %d %d  \n", op1, tipo1);
    if (BitZ(maquina->registros[CC])) // bit cero en 1
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JP(MV *maquina, int op1, int tipo1)
{
    printf("jp %d %d  \n", op1, tipo1);
    if (!BitN(maquina->registros[CC]) && !BitZ(maquina->registros[CC])) // bit signo en 0 y bit cero en 0
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JN(MV *maquina, int op1, int tipo1)
{
    printf("jp %d %d  \n", op1, tipo1);
    if (BitN(maquina->registros[CC])) // bit signo en 1
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JNZ(MV *maquina, int op1, int tipo1)
{
    printf("jnz %d %d  \n", op1, tipo1);
    if (!BitZ(maquina->registros[CC])) // bit cero en 0
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JNP(MV *maquina, int op1, int tipo1)
{
    printf("jnp %d %d \n", op1, tipo1);
    if (BitN(maquina->registros[CC]) || BitZ(maquina->registros[CC])) // no positivo
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void JNN(MV *maquina, int op1, int tipo1)
{
    printf("jnn %d %d \n", op1, tipo1);
    if (!BitN(maquina->registros[CC]) || BitZ(maquina->registros[CC])) // no negativo
        maquina->registros[IP] = GetValor(maquina, op1, tipo1);
    else
        maquina->registros[IP]++;
}

void LDH(MV *maquina, int op1, int tipo1)
{
    printf("ldh %08x %d  \n", op1, tipo1);
    int aux;
    if (tipo1 == 0x01)
    {
        aux = GetValor(maquina, op1, tipo1) & 0xFFFF; // registro tiene 4 bytes, guardo solo los ultimos 2.
    }
    else if (tipo1 == 0x02)
    {
        aux = maquina->memoria[maquina->registros[DS] + op1] & 0xFFFF; // celda tiene 4 bytes, guardo solo los ultimos 2.
    }
    else
    {
        aux = op1 << 16; //<< 16; // op inmediato ya es de 16 bits (2bytes)
    }
    maquina->registros[AC] = (maquina->registros[AC] & ~0xFFFF0000);
    maquina->registros[AC] = maquina->registros[AC] | (aux & 0xFFFF0000); // cambio primeros dos bytes de AC q tiene 4 bytes
    maquina->registros[IP]++;
}

void LDL(MV *maquina, int op1, int tipo1)
{
    printf("ldl %d %d  \n", op1, tipo1);
    int aux;
    if (tipo1 == 0x01)
    {
        aux = GetValor(maquina, op1, tipo1) & 0xFFFF; // registro tiene 4 bytes, guardo solo los ultimos 2.
    }
    else if (tipo1 == 0x02)
    {
        aux = maquina->memoria[maquina->registros[DS] + op1] & 0xFFFF; // celda tiene 4 bytes, guardo solo los ultimos 2.
    }
    else
    {
        aux = op1; //<< 16; // op inmediato ya es de 16 bits (2bytes)
    }
    maquina->registros[AC] = (maquina->registros[AC] & ~0xFFFF);
    maquina->registros[AC] = maquina->registros[AC] | (aux & 0xFFFF); // cambio ULT dos bytes de AC q tiene 4 bytes
    maquina->registros[IP]++;
}

void NOT(MV *maquina, int op1, int tipo1)
{
    printf("not %d %d  \n", op1, tipo1);
    int aux = ~GetValor(maquina, op1, tipo1);
    SetValor(maquina, op1, tipo1, aux);
    ActualizarCC(maquina, aux);
    maquina->registros[IP]++;
}

void STOP(MV *maquina)
{
    printf("stop");
    maquina->registros[IP] = maquina->registros[DS] + 1;
}

void RND(MV *maquina, int op1, int tipo1)
{
    printf("rnd %d %d \n", op1, tipo1);
    srand(time(0)); // hace que varie en cada ejecucion del programa, sino siempre es la misma secuencia
    SetValor(maquina, AC, REGISTRO, rand() % (GetValor(maquina, op1, tipo1) + 1));
    maquina->registros[IP]++;
}

void PUSH(MV *maquina, int op1, int tipo1)
{
    maquina->registros[SP]--;
    int pos_memoria = (maquina->registros[SS] && 0xFF) + maquina->registros[SP];
    maquina->memoria[pos_memoria] = GetValor(maquina, op1, tipo1);
}
void POP(MV *maquina, int op1, int tipo1)
{
    int pos_memoria = (maquina->registros[SS] && 0xFF) + maquina->registros[SP];
    SetValor(maquina, op1, tipo1, maquina->memoria[pos_memoria]);
    maquina->registros[SP]++;
}

void CALL(MV *maquina, int op1, int tipo1)
{
    PUSH(maquina, maquina->registros[IP], INMEDIATO);
    maquina->registros[IP] = GetValor(maquina, op1, tipo1);
}

void RET(MV *maquina)
{
    int pos_memoria = (maquina->registros[SS] && 0xFF) + maquina->registros[SP];
    maquina->registros[SP]++;
    int val = GetValor(maquina, pos_memoria, MEMORIA);
    maquina->registros[IP] = val;
}

void SYS(MV *maquina, int op1, int tipo1)
{
    printf("Sys \n");

    switch (op1)
    {
    case 0x1:
    {
        // printf("read\n");
        // read
        int mem_start = GetValor(maquina, D, REGISTRO);
        int n = GetValor(maquina, 0x3C, REGISTRO);
        int config = GetValor(maquina, 0x3A, REGISTRO);
        int write_prompt = !(config & 0x800);
        int aux = 0;

        if (write_prompt && config & 0x100)
        {
            printf("[%04d]: ", mem_start);
        }
        for (int i = 0; i < n; i++)
        {

            if (config & 0x100)
            {
                aux = getchar();
                if (aux == '\n' || aux == '\0')
                    break;
            }
            else
            {
                if (write_prompt)
                {
                    printf("[%04d]: ", mem_start + i);
                }
                scanf((config & 0x008) ? "%x" : ((config & 0x004) ? "%o" : "%d"), &aux);
            }
            SetValor(maquina, mem_start + i, MEMORIA, aux);
        }
        break;
    }

    case 0x2:
    {
        // printf("write\n");
        // write
        int mem_start = GetValor(maquina, D, 0x01); //tendria que ser indireccion?
        int n = GetValor(maquina, 0x3C, 0x01);
        int config = GetValor(maquina, 0x3A, 0x01);
        int write_prompt = !(config & 0x800);
        int new_line = !(config & 0x100);
        int val = 0;
        for (int i = 0; i < n; i++)
        {
            val = GetValor(maquina, mem_start + i, MEMORIA);
            if (write_prompt)
            {
                printf("[%04d]: ", mem_start + i);
            }

            config & 0x10 ? printf("%c ", val & 0xFF) : printf(" ");
            config & 0x08 ? printf("%%%X ", val) : printf(" ");
            config & 0x04 ? printf("@%o ", val) : printf(" ");
            config & 0x01 ? printf("%d ", val) : printf(" ");
            if (new_line)
                printf("\n");
        }

        break;
    }
    case 0x3:{ //LEER CADENAS

        //INDIRECCION                                                           //  EDX=0x00020019
        int buffer= GetValor(maquina,D,REGISTRO);                               //  ES=00300010
        int codSeg=(buffer>>16) & 0xFFFF; //parte alta
        int desp=buffer & 0xFFFF; //parte baja                                  // DIR ABSOLUTA= 10 + 19 = 29
        int posSeg=(maquina->registros[codSeg])&0xFFFF;
        int dirAbs=posSeg + desp;


        int n=GetValor(maquina,0x3C,REGISTRO);

        //RECOLECTA FORMATO
        int config=GetValor(maquina,0x3A,REGISTRO);
        int prompt = (config & 0x800)>>11; //Verifica el bit 11 del AX


        char* aux=NULL;
        int i=0;
        if(prompt==0)
            printf("[%04d]: ", dirAbs);
        scanf("%s",&aux);

        while(i<n){
            if(i==n-1){     //guarda \0 en la ultima posicion
                maquina->memoria[dirAbs+i]='\0';
            }
            else{
                maquina->memoria[dirAbs+i]= aux[i];
                if (aux[i] == '\n' || aux[i] == '\0')
                    i=n;
            }
            i++;
        }
    }
        break;
    case 0x4:{ //ESCRIBIR CADENAS

        //INDIRECCION
        int buffer= GetValor(maquina,D,REGISTRO);
        int codSeg=(buffer>>16) & 0xFFFF; //parte alta
        int desp=buffer & 0xFFFF; //parte baja
        int posSeg=(maquina->registros[codSeg])&0xFFFF;
        int dirAbs=posSeg + desp;

        //RECOLECTA TAMAÑO DE CADENA
        int n=GetValor(maquina,0x3C,REGISTRO);

         //RECOLECTA FORMATO
        int config=GetValor(maquina,0x3A,REGISTRO);
        int prompt = (config & 0x800)>>11; //Verifica el bit 12 del AX
        int endln=(config&0x100)>>8; //Verifica el bit 9 del AX

        int i=0;
        while(i<n){
            if(prompt==0)
                printf("[%04d]: ", dirAbs);
            printf("%c",maquina->memoria[dirAbs+i]&0xFF);
            if(endln==0)
                printf("\n");
            if(maquina->memoria[dirAbs+i]&0xFF == '\n')
                i=n;
            i++;
        }

    }
        break;

    case 0xD:{ //VDD
            int op= GetValor(maquina,0x2A,REGISTRO); //operacion
            int cant=GetValor(maquina,0x1A,REGISTRO);//cantidad de sectores a leer

            //obtiene posicion inicial
            int c=GetValor(maquina,0x2C,REGISTRO); //cilindro
            int h=GetValor(maquina,0x1C,REGISTRO); //cabeza
            int s=GetValor(maquina,0x2D,REGISTRO); //sector
            int id=GetValor(maquina,0x1D,REGISTRO); //unidad de disco

            int buffer= GetValor(maquina,B,REGISTRO);

            if(strcmp(maquina->discos[id].nomArch,"")!=0){ //VERIFICA QUE EXISTA EL DISCO

                if(op==0x08){ //PARAMETROS
                    SetValor(maquina,0x2C,REGISTRO,maquina->discos[id].cilindros);
                    SetValor(maquina,0x1C,REGISTRO,maquina->discos[id].cabezas);
                    SetValor(maquina,0x2D,REGISTRO,maquina->discos[id].sectores);
                    SetValor(maquina,0x2A,REGISTRO,0x00);
                    printf("Operacion exitosa\n");
                }else if(op==0x00){ //CONSULTA ESTADO
                    SetValor(maquina,0x2A,REGISTRO,maquina->discos[id].estado); //carga el estado de la ultima operacion del disco en el AH
                }else{

                    //VALIDACIONES
                    if(maquina->discos[id].cilindros<=c){ //nro de cilindro supera al maximo
                        SetValor(maquina,0x2A,REGISTRO,0x0B);
                        printf("Numero invalido de cilindro\n");
                    }else if(maquina->discos[id].cabezas<=h){ //nro de cabeza supera al maximo
                        SetValor(maquina,0x2A,REGISTRO,0x0C);
                        printf("Numero invalido de cabeza\n");
                    }else if(maquina->discos[id].sectores<=s){ //nro de sector supera al maximo
                        SetValor(maquina,0x2A,REGISTRO,0x0D);
                        printf("Numero invalido de sector\n");
                    }else{
                        if(op==0x03) //ESCRITURA
                            escrituraVDD(maquina->discos[id],c,h,s,buffer,cant,maquina);
                        else if(op==0x02) //LECTURA
                            lecturaVDD(maquina->discos[id],c,h,s,buffer,cant,maquina);
                        else{ //SE INVOCA UNA FUNCION FUERA DE LAS 4 PERMITIDAS
                            SetValor(maquina,0x2A,REGISTRO,0x01);
                            printf("Numero invalido de sector\n");
                        }
                    }
                    maquina->discos[id].estado=GetValor(maquina,0x2A,REGISTRO);
                }
            }else{
                SetValor(maquina,0x2A,REGISTRO,0x31);
                printf("No existe el disco\n");
            }
        }
        break;
    case 0x7:
        ClearScreen();
        break;
    case 0xF:
    {
        // breakpoint
        if (maquina->flag_b)
        {
            maquina->flag_break = 1;
        }

        break;
    }
    }
    maquina->registros[IP]++;
}

int posicionVDD(int h, int c, int s,VDD disco){
    int HD=disco.header; //tamaño del header
    int C=disco.cilindros; //cant cilindros
    int S=disco.sectores; //cant sectores
    int TS=disco.tSector; //tamaño del sector

    return HD + c*C*S*TS + h*S*TS + s*TS;
}

void lecturaVDD(VDD disco,int c, int h, int s, int buffer, int cant,MV* maquina){
    FILE *arch=fopen(disco.nomArch,"rb");
    int aux= (buffer>>16)&0xFFFF; //codigo de registro
    int offset= buffer&0xFFFF; //desplazamiento

    int reg= maquina->registros[aux];//registro
    int dirAbs=reg&0xFFFF + offset; //direccion del registro + desplazamiento
    int celdas= (disco.tSector*cant)/4;
    int datoSector[128]; //variable para almacenar los datos del sector

    //VERIFICA QUE ENTRE EN EL ESPACIO DE MEMORIA DEL SEGMENTO INDICADO
    if (dirAbs+celdas<=(reg&0xFFFF)+(reg>>16)&0xFFFF){

        fseek(arch,posicionVDD(h,c,s,disco),SEEK_SET);//posicion inicial de lectura
        int sectores=1; //cantidad de sectores leidos

        while (sectores<=cant){
            fread(datoSector,sizeof(int),128,arch); //LEE TODO UN SECTOR
            for (int i=0;i<128;i++) //ALMACENA LOS DATOS EN LA MEMORIA
                maquina->memoria[dirAbs++]=datoSector[i];

            //UNA VEZ LEIDO EL SECTOR, PASA AL SIGUIENTE Y COMPRUEBA QUE SIGA EN LA CABEZA CORRECTA
            if(++s==disco.sectores){ //ya no hay sectores en la cabeza
                h++;    //avanza a la siguiente cabeza
                s=0;
                if(h==disco.cabezas){ //supera cantidad de cabezas
                    c++;
                    h=0;
                    if(c==disco.cilindros){ //supera el tamaño del disco
                        SetValor(maquina,0x1A,REGISTRO,sectores);
                        sectores=cant+1; //corta la lectura
                    }
                }
            }
            fseek(arch,posicionVDD(h,c,s,disco),SEEK_SET);//posicion inicial de lectura
            sectores++;
        }
        SetValor(maquina,0x2A,REGISTRO,0x00);
        printf("Operacion exitosa\n");
    }else{
         SetValor(maquina,0x2A,REGISTRO,0x04);
        printf("Error de lectura\n");
    }
    fclose(arch);
}
void escrituraVDD(VDD disco,int c,int h, int s,int buffer, int cant, MV* maquina){
    FILE *arch=fopen(disco.nomArch,"ab");
    int aux= (buffer>>16)&0xFFFF; //codigo de registro
    int offset= buffer&0xFFFF; //desplazamiento

    int reg= maquina->registros[aux];//registro
    int dirAbs=reg&0xFFFF + offset; //direccion del registro + desplazamiento
    int celdas= (disco.tSector*cant)/4;
    int datoSector[128]; //variable para almacenar los datos que se cargaran
    int error=0;
    //VERIFICA QUE ENTRE EN EL ESPACIO DE MEMORIA DEL SEGMENTO INDICADO
    if (dirAbs+celdas<=(reg&0xFFFF)+(reg>>16)&0xFFFF){

         fseek(arch,posicionVDD(h,c,s,disco),SEEK_SET);//posicion inicial de lectura
        int sectores=1; //cantidad de sectores leidos

        while (sectores<=cant){
            for (int i=0;i<128;i++) //ALMACENA LOS DATOS DE LA MEMORIA
                datoSector[i]=maquina->memoria[dirAbs++];

            fwrite(datoSector,sizeof(int),128,arch); //CARGA TODO UN SECTOR

            //UNA VEZ CARGADO EL SECTOR, PASA AL SIGUIENTE Y COMPRUEBA QUE SIGA EN LA CABEZA CORRECTA
            if(++s==disco.sectores){ //ya no hay sectores en el cilindro
                h++;    //avanza a la siguiente cabeza
                s=0;
                if(h==disco.cabezas){ //supera cantidad de cabezas
                    c++;
                    h=0;
                    if(c==disco.cilindros){ //supera el tamaño del disco
                        SetValor(maquina,0x2A,REGISTRO,0xFF);
                        error=1;
                        sectores=cant+1; //corta la lectura
                    }

                }
            }
            fseek(arch,posicionVDD(h,c,s,disco),SEEK_SET);//posicion inicial de lectura
            sectores++;
        }
        if (!error){
            SetValor(maquina,0x2A,REGISTRO,0x00);
            printf("Operacion exitosa\n");
        }
    }else{
         SetValor(maquina,0x2A,REGISTRO,0xCC);
        printf("Error de escritura\n");
    }
    fclose(arch);
}

void Breakpoint(MV *maquina)
{
    maquina->flag_break = 0;
    if (maquina->flag_c)
    {
        ClearScreen();
    }
    if (maquina->flag_d)
    {
        Disassemble(*maquina);
    }
    printf("[%04d] cmd: ", maquina->registros[IP]);
    char buffer[10]; // maxima cantidad de caracteres validos es direccion de 4 digitos, espacio , otra direccion de 4 digitos y /n (10).
    fgets(buffer, 10, stdin);
    int nl = EncontrarChar(buffer, 10, '\n');
    if (nl == -1)
    {
        printf("Comando invalido. \n");
    }
    else
    {
        if (nl == 1)
        {
            if (buffer[0] == 'p')
            {
                maquina->flag_break = 1;
            }
            else if (buffer[0] != 'r')
            {
                printf("Comando invalido. \n");
            }
        }
        else
        {
            int mem_start = -1, mem_end = -1;
            int espacio = EncontrarChar(buffer, 10, ' ');
            if (espacio == -1)
            {
                mem_start = GetNum(buffer, nl);
                mem_end = mem_start;
            }
            else
            {
                mem_start = GetNum(buffer, espacio);
                mem_end = GetNum(&buffer[espacio + 1], nl - espacio - 1);
            }
            if (mem_start == -1 || mem_end == -1)
            {
                printf("Comando invalido. \n");
            }
            else
            {
                MostrarRangoMemoria(*maquina, mem_start, mem_end);
            }
        }
    }
}

F0 Instrucciones_0[2] = {&RET, &STOP};
F1 Instrucciones_1[15] = {&SYS, &JMP, &JZ, &JP, &JN, &JNZ, &JNP, &JNN, &LDL, &LDH, &RND, &NOT, &PUSH, &POP, &CALL};
F2 Instrucciones_2[15] = {&MOV, &ADD, &SUB, &SWAP, &MUL, &DIV, &CMP, &SHL, &SHR, &AND, &OR, &XOR, &SLEN, &SMOV, &SCMP};
