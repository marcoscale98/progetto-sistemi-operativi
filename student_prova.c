#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include "header/shm_util.h"
#include "header/error.h"
#include "header/config.h"
#include "header/sig_util.h"
#include "header/sem_util.h"
#include "header/stud.h"

#define NOGROUP -1
#define EMPTY -1

#define FREE 0
#define MEMBER 1
#define LEADER 2

struct info_student{
    int matricola;  //matricola dello studente associato alla struttura
    int nof_elems;  //nof_elems dello studente
    int voto_AdE;   //voto_AdE dello studente
    int group;     //gruppo di cui lo studente fa parte (NOGROUP oppure indice dell'array group)
};

struct info_group{
    int members[4];  //elementi del gruppo: ogni elemento corrisponde ad una matricola. EMPTY=vuoto
    int number; //numero di elementi
    int is_closed;  //false (aperto), true (chiuso)
};

//struttura che andrà in MEMORIA CONDIVISA
struct info_sim { 
    struct info_student student[POP_SIZE];
    struct info_group group[POP_SIZE]; //EMPTY=vuoto
    int time_left;
};

//handler per SIGUSR1: per processi studente
void handler_sigusr1(int sig){
    printf("Student (PID: %d). Bloccato. Aspetto il voto\n",getpid());
    int msg_id;
    msg_id = msgget(IPC_KEY,0666);
    TEST_ERROR;

    struct msgbuf message;
    msgrcv(msg_id,&message,sizeof(message.text),student->matricola,0);
    printf("Student (PID: %d). voto_SO: %d\n", getpid(), atoi(message.text));

    //FINE SIMULAZIONE
    shmdt(shared);

    exit(EXIT_SUCCESS);
}

//funzione richiamata per impostare l'handler di SIGALRM
int sa_sigusr1(){
    struct sigaction sa;
    sa.sa_handler = handler_sigusr1;
    return sigaction(SIGUSR1,&sa,NULL);
}

int choose_student(struct info_sim* shm,)

int main(int argc,char *argv[]){
    //set handler
    sa_sigusr1();
    TEST_ERROR;
    sa_sigsegv(); //può capitare il segmentation fault nello student
    TEST_ERROR;

    if(argc!=6){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }

    //argv[0]="student", argv[1]=matricola, argv[2]=prob_2, argv[3]=prob_3, argv[4]=nof_invites, argv[5]=max_reject
    int nof_invites, max_reject;
    float prob_2, prob_3;
    matricola = atoi(argv[1]);
    prob_2 = atoi(argv[2])/100.0;
    prob_3 = atoi(argv[3])/100.0;
    max_reject = atoi(argv[5]);

    //INIZIALIZZAZIONE IPC
    int sem_id, shm_id;
    sem_id = semget(IPC_KEY, N_SEM, 0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY, 0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE, 0666);
    TEST_ERROR;

    //INIZIALIZZAZIONE MEMORIA CONDIVISA
    shared = (struct info_sim *)shmat(shm_id, NULL, 0666);
    TEST_ERROR;

    struct info_student *this_student = shared->student[matricola];
    this_student->matricola = matricola;
    this_student->group = NOGROUP;
    
    //inizializzazione nof_elems
    srand(getpid());
    int val = rand()%POP_SIZE;
    if(val<POP_SIZE*prob_2)
        this_student->nof_elems = 2;
    else if(val>=POP_SIZE*prob_2 && val<POP_SIZE*(prob_2+prob_3))
        this_student->nof_elems = 3;
    else //if(val>=popsize*(prob_2+prob_3) && val<popsize)
        this_student->nof_elems = 4;
    
    //inizializzazione voto_AdE
    this_student->voto_AdE = rand()%13 + 18;  //compreso tra 18 e 30//inizializzazione voto_AdE
  
    //fine inizializzazione
    reserve_sem(sem_id, SEM_READY);
    TEST_ERROR;

    //
    //  INIZIO STRATEGIA
    //

    int state = FREE,
        last_invite,
        my_invitations = 0,
        my_rejects = 0;
    struct msgbuf message;
    memset(invites,-1,sizeof(invitati));

    while(shared->time_left > 0){
        //se non ha messaggi da leggere
        if(msgrcv(msg_id, &message, sizeof(message.text), this_student->matricola, IPC_NOWAIT)==-1){
            //se non ha raggiunto il massimo di inviti 
            // e il suo gruppo non ha gia' raggiunto il numero di membri preferito
            // e non e' membro di un altro gruppo
            if(my_invitations < nof_invites &&
               (this_student->group == NOGROUP || shared->group[this_student->group].number < nof_elems) &&
               state!=MEMBER){

                //AGGIUNGERE CONTROLLO SULLO STESSO GRUPPO

                message.mtype = choose_student(shared,last_invite);     //da implementare
                sprintf(message.text,"%d invite",this_student->matricola);
                msgsnd(msg_id,&message,sizeof(message.text),0);
                TEST_ERROR;
                my_invitations++;
            } //altrimenti non invita
        } else { //se ha messaggi da leggere
            char buf[15];
            int sender;
            sscanf(message.text,"%d %s",buf,&sender);
            //se e' un invito
            if(strcmp(buf,"invite")==0){
                //se non e ne' leader ne' membro o il suo gruppo e chiuso
                // o non e' la risposta all'ultimo invito
                if(state==LEADER || state==MEMBER ||
                   (this_student.group=!NOGROUP && shared->group[this_student->group].is_closed) ||
                   sender!=last_invite){
                    message.mtype = sender;
                    sprintf(message.text,"%d denied",this_student->matricola);
                    msgsnd(msg_id,&message,sizeof(message.text),0);
                    TEST_ERROR;
                //valuta la risposta
                } else {
                    //se e' conveniente accettare e il numero di rifiuti e' minore del massimo
                    if(accept_invite(shared,sender) && my_rejects<max_reject){      //da implementare
                        message.mtype = sender;
                        sprintf(message.text,"%d accepted",this_student->matricola);
                        msgsnd(msg_id,&message,sizeof(message.text),0);
                        TEST_ERROR;
                        state = MEMBER;
                    } else { //se rifiuta
                        message.mtype = sender;
                        sprintf(message.text,"%d denied",this_student->matricola);
                        msgsnd(msg_id,&message,sizeof(message.text),0);
                        TEST_ERROR;
                        my_rejects++;
                    }
                }
            //se ha risposte agli inviti, non e' un membro di un gruppo
            // e il suo gruppo non e' chiuso
            } else if(state!=MEMBER &&
                      (this_student->group == NOGROUP || !shared->group[this_student->group].is_closed)){
                //se e' stato accettato un invito
                if(strcmp(buf,"accepted")==0){
                    //INIZIO SEZIONE CRITICA
                    int group = group_add(shared,sender);       //da implementare
                    this_student->group = group;
                    shared->group[this_student->group].number++;
                    //se ha raggiunto il numero di membri preferiti chiudi il gruppo
                    if(shared->group[this_student->group].number == this_student->nof_elems){
                        shared->group[this_student->group].is_closed = TRUE;
                    }
                    //FINE SEZIONE CRITICA
                    state = LEADER;
                }
            }
        }
    }
}