#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "header/stud.h"

#define NOGROUP 0

int main(int argc, char *argv[]){ /*argv[0]="student", argv[1]=matricola, argv[2]=prob_2, argv[3]=prob_3,
    argv[4]=nof_invites, argv[5]=max_reject, argv[6]=POPSIZE */
    
    if(argc!=7){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }
    
    //definizione e inizializzazione variabili studente    
    struct info_student *stud;
    float prob_2= atof(argv[2]),
        prob_3= atof(argv[3]);
    int nof_invites = atoi(argv[4]),
        max_reject = atoi(argv[5]),
        popsize = atoi(argv[6]);
    
    stud->matricola = atoi(argv[1]);
    stud->group = NOGROUP; //non ancora in un gruppo
    //l'indice dei gruppi parte dall'1
    
    //inizializzazione voto_AdE
    srand(getpid());
    stud->voto_AdE = rand()%13 + 18;  //compreso tra 18 e 30

    //inizializzazione nof_elems
    srand(getpid());
    int val = rand()%popsize;
    if(val<popsize*prob_2)
        stud->nof_elems = 2;
    else if(val>=popsize*prob_2 && val<popsize*(prob_2+prob_3))
        stud->nof_elems = 3;
    else //if(val>=POPSIZE*(prob_2+prob_3) && val<POPSIZE)
        stud->nof_elems = 4;
    
    printf(" stud->matricola: %d\n", stud->matricola);
    printf(" prob_2: %f\n", prob_2);
    printf(" prob_3: %f\n", prob_3);
    printf(" nof_invites: %d\n", nof_invites);
    printf(" max_reject: %d\n", max_reject);
    printf(" popsize: %d\n", popsize);
    printf(" stud->group: %d\n", stud->group);
    printf(" stud->voto_AdE: %d\n", stud->voto_AdE);
    printf(" stud->nof_elems: %d\n", stud->nof_elems);
        
    //incremento del semaforo di 1
    
    return 0;
}
