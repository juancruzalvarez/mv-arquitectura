#ifndef MAQUINAVIRTUAL_H_INCLUDED
#define MAQUINAVIRTUAL_H_INCLUDED

#define CANT_REGISTROS 16
#define CELDAS_MEMORIA 8192


enum REGISTROS{
    DS = 0,SS,ES,CS,HP,
    IP,SP,BP,
    CC,
    AC,A,B,C,D,E,F
};


typedef struct MV{
    int registros[CANT_REGISTROS];
    int memoria[CELDAS_MEMORIA];
    int flag_break;
    int flag_b;
    int flag_c;
    int flag_d;
}MV;



#endif // MAQUINAVIRTUAL_H_INCLUDED
