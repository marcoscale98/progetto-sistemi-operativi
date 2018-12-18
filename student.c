#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "header/stud.h"

#define NOGROUP 0

int main(int argc, char *argv[]){ //argv[0]="student", argv[1]=matricola, argv[2]=prob_2, argv[3]=prob_3, \
    argv[4]=prob_4, argv[5]=max_reject, argv[6]=POPSIZE
    
    if(argc!=6){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }
    
    //definizione e inizializzazione variabili studente    
    struct info_student stud;
    float prob2= atof(argv[2]),
        prob3= atof(argv[3]),
        prob4= atof(argv[4]);
    int max_reject = atoi(argv[5]),
        popsize = atoi(argv[6]);
    
    stud->matricola = atoi(argv[1]);
    stud->group = NOGROUP; //non ancora in un gruppo
    //l'indice dei gruppi parte dall'1
    
    //inizializzazione voto_AdE
    srand(getpid());
    stud->voto_AdE = rand()%13 + 18;  //compreso tra 18 e 30

    //inizializzazione nof_elems
    srand(getpid());
    int val = rand()%POPSIZE;
    if(val<POPSIZE*prob_2)
        stud->nof_elems = 2;
    else if(val>=POPSIZE*prob_2 && val<POPSIZE*(prob_2+prob_3))
        stud->nof_elems = 3;
    else //if(val>=POPSIZE*(prob_2+prob_3) && val<POPSIZE)
        stud->nof_elems = 4;
        
    //incremento del semaforo di 1
    
}
