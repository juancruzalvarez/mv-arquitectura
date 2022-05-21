#include <stdio.h>
#include <stdlib.h>

int main()
{
    int hola[128];
    int prueba[128];
    for(int i=0; i<128; i++)
        hola[i]=i;
    FILE* arch;
    arch=fopen("prueba.txt","wb");
    if (arch){
        fwrite(hola,sizeof(int),128,arch);
        printf("archivo creado\n");
    }
    fclose(arch);
    arch=fopen("prueba.txt","rb");
    if (arch){
        fread(prueba,sizeof(int),128,arch);
        printf("archivo leido\n");
    }
     for(int i=0; i<128; i++)
        printf("%d \n",prueba[i]);
    fclose(arch);
}
