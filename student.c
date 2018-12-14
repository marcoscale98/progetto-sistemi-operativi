#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]){
    int voto_AdE, voto_SO, matricola;

    if(argc!=2){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }

    printf("i = %s\n", argv[1]);
    
    srand(getpid());
    voto_AdE = rand()%13 + 18;      //compreso tra 18 e 30


    //incremento del semaforo di 1
}