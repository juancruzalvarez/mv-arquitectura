#ifndef MAQUINAVIRTUAL_H_INCLUDED
#define MAQUINAVIRTUAL_H_INCLUDED

#define CANT_REGISTROS 16
#define CELDAS_MEMORIA 8192
#define CANT_DISCOS 255 //PREGUNTAR


enum REGISTROS{
    DS = 0,SS,ES,CS,HP,
    IP,SP,BP,
    CC,
    AC,A,B,C,D,E,F
};

typedef struct{
    int header;
    int ID; //nro de unidad
    char* nomArch; //nombre archivo fisico real
    int cilindros; //cantidad de cilindros
    int cabezas;
    int sectores;
    int tSector; //tama�o del sector
    int estado;
}VDD;

typedef struct MV{
    int registros[CANT_REGISTROS];
    int memoria[CELDAS_MEMORIA];
    VDD discos[CANT_DISCOS];
    int flag_break;
    int flag_b;
    int flag_c;
    int flag_d;
}MV;



#endif // MAQUINAVIRTUAL_H_INCLUDED
