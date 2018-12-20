#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "header/stud.h"

#define NOGROUP 0

int main(int argc, char *argv[]){ /*argv[0]="student", argv[1]=matricola, argv[2]=prob_2, argv[3]=prob_3, argv[4]=nof_invites, argv[5]=max_reject, argv[6]=POPSIZE, argv[7]=shmid */
    
    if(argc!=8){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }
    
    //definizione e inizializzazione variabili studente    
    struct info_student info_stud;
    float prob_2 = atoi(argv[2])/100.0,
          prob_3 = atoi(argv[3])/100.0;
    int popsize = atoi(argv[6]),
        shmid = atoi(argv[7]),
        nof_invites = atoi(argv[4]),
        max_reject = atoi(argv[5]);
        
    info_stud.matricola = atoi(argv[1]);
    info_stud.group = NOGROUP; //non ancora in un gruppo
    //l'indice dei gruppi parte dall'1
    
    //inizializzazione voto_AdE
    srand(getpid());
    info_stud.voto_AdE = rand()%13 + 18;  //compreso tra 18 e 30

    //inizializzazione nof_elems
    srand(getpid());
    int val = rand()%popsize;
    if(val<popsize*prob_2)
        info_stud.nof_elems = 2;
    else if(val>=popsize*prob_2 && val<popsize*(prob_2+prob_3))
        info_stud.nof_elems = 3;
    else //if(val>=POPSIZE*(prob_2+prob_3) && val<POPSIZE)
        info_stud.nof_elems = 4;
    
    //TESTING
    /*
    printf(" info_stud.matricola: %d\n", info_stud.matricola);
    printf(" prob_2: %f\n", prob_2);
    printf(" prob_3: %f\n", prob_3);
    printf(" nof_invites: %d\n", nof_invites);
    printf(" max_reject: %d\n", max_reject);
    printf(" popsize: %d\n", popsize);
    printf(" info_stud.group: %d\n", info_stud.group);
    printf(" info_stud.voto_AdE: %d\n", info_stud.voto_AdE);
    printf(" info_stud.nof_elems: %d\n", info_stud.nof_elems);
    printf(" shmid: %d\n", shmid);
    */
        
    //incremento del semaforo di 1
    
    return 0;
}
