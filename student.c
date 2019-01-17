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

//
//
//  IMPLEMENTARE IL CRITIC TIME
//
//

//definizione simbolica di 'non appartiene a nessun gruppo'
#define NOGROUP -1

//valori che la variabile stato puo' assumere
#define FREE 0
#define MEMBER 1
#define LEADER 2

//valori che array_invites puo' assumere
#define AVAILABLE 0
#define WAIT_ANSW 1
#define ANSWERED 2

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

//funzione richiamata per impostare l'handler di SIGUSR1
int sa_sigusr1(){
    struct sigaction sa;
    sa.sa_handler = handler_sigusr1;
    return sigaction(SIGUSR1,&sa,NULL);
}

//funzione che restituisce TRUE se aspetta risposte
int waiting_answers(int *array){
    if(array){
        int i, cnt;
        for(i=0,cnt=0;i<POP_SIZE;i++){
            if(array[i]==WAIT_ANSW)
                cnt++;
        }
        return cnt>0;
    } else
    return -1;
}

//funzione che ritorna lo studente da invitare nel caso di successo, -1 altrimenti
int choose_student(struct info_sim* shm, int* array, int my_matricola){
    if(shm && array && my_matricola>=0){
        struct info_student this_student = shm->student[my_matricola];
        int i;
        //se il voto dello studente e' compreso tra 27 e 30
        if(this_student.voto_AdE>=27){
            for(i=0;i<POP_SIZE;i++){
                //se non sono io e lo studente e' dello stesso gruppo
                // e lo studente ha lo stesso nof_elems e non l'ho gia' invitato
                if(i!=my_matricola && i%2==my_matricola%2 &&
                   shm->student[i].nof_elems==this_student.nof_elems &&
                   array[i]==AVAILABLE)
                    return i;
            }
            //se non ha trovato uno studente ritorna -1
            return -1;
        //se il voto dello studente e' compreso tra 22 e 26
        } else if(this_student.voto_AdE>=22) {
            for(i=0;i<POP_SIZE;i++){
                //se non sono io e lo studente e' dello stesso gruppo
                // e lo studente ha voto maggiore e non l'ho gia' invitato
                if(i!=my_matricola && i%2==my_matricola%2 &&
                   shm->student[i].nof_elems==this_student.nof_elems &&
                   array[i]==AVAILABLE)
                    return i;
            }
            //se non ha trovato uno studente ritorna -1
            return -1;
        //se il voto dello studente e' compreso tra 18 e 21
        } else {
            for(i=30;i>=18;i--){
                int j;
                for(j=0;j<POP_SIZE;j++){
                    //se non sono io e lo studente e' dello stesso gruppo
                    // e esiste uno studente con voto 'i' e non l'ho gia' invitato
                    if(j!=my_matricola && j%2==my_matricola%2 &&
                       shm->student[j].voto_AdE==i && array[j]==AVAILABLE)
                        return i;    
                }
            }
            //se non ha trovato uno studente ritorna -1
            return -1;
        }
    //in caso di errore ritorna -1
    } else
    return -1;
}

//funzione che restituisce TRUE se lo studente accetta il voto e FALSE altrimenti
// modifica il campo group se lo studente ha accettato
int accept_invite(struct info_sim *shm, int sender,int my_matricola){
    if(shm && sender>=0 && my_matricola>=0){
        struct info_student *this_student = shm->student[my_matricola]
                            inviter = shm->student[sender];
        //se il voto dello studente e' compreso tra 27 e 30
        if(this_student.voto_AdE>=27){
            //accetta se lo studente che lo ha invitato ha nof_elems uguale al suo
            if(inviter.nof_elems==this_student.nof_elems){
                this_student->group = inviter.group;
                return TRUE;
            } else
                return FALSE;
        //se il voto dello studente e' compreso tra 18 e 26
        } else {
            //accetta se lo studente che lo ha invitato ha voto maggiore al suo
            if(inviter.voto_AdE>this_student.voto_AdE){
                this_student->group = inviter.group;
                return TRUE;
            } else
                return FALSE;
        }
    //in caso di errore ritorna -1
    } else
    return -1;
}


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

    // ######### INIZIO STRATEGIA #########
    init_sem_available(sem_id,this_student->matricola);

    int state = FREE,
        *array_invites[POP_SIZE],
        my_invitations = 0,
        my_rejects = 0,
        i;
    for(i=0;i<POP_SIZE;i++)
        array_invites[i]=AVAILABLE;
    struct msgbuf message;

    //finche' non scade il tempo
    while(shared->time_left > 0){
        //se manca poco riprovo ad invitare i vecchi invitati
        if(shared->time_left<=CRITIC_TIME){
            for(i=0;i<POP_SIZE;i++)
                array_invites[i]=AVAILABLE;
        }
        // effettuando la reserve sul semaforo con la sua matricola
        // nessuno studente lo puo' invitare mentre sta operando
        reserve_sem(sem_id,this_student->matricola);
        //se non ha messaggi da leggere
        if(msgrcv(msg_id, &message, sizeof(message.text), this_student->matricola+100, IPC_NOWAIT)==-1){
            //se non ha raggiunto il massimo di inviti 
            // e il suo gruppo non ha gia' raggiunto il numero di membri preferito
            // e non e' membro di un altro gruppo
            if(my_invitations < nof_invites &&
               (this_student->group == NOGROUP || shared->group[this_student->group].number < nof_elems) && //o e' chiuso
               state!=MEMBER){
                int student_invited = choose_student(shared,last_invite)+100;
                //se ho trovato uno studente da invitare
                if(student_invited!=-1){
                    message.mtype = student_invited+100;
                    sprintf(message.text,"%d invite",this_student->matricola);
                    
                    //effettuo la reserve sul semaforo con la matricola dello studente che ho scelto
                    reserve_sem(sem_id,student_invited);
                    // ############### SEZIONE CRITICA ###########################
                    msgsnd(msg_id,&message,sizeof(message.text),0);
                    TEST_ERROR;
                    // ############### FINE SEZIONE CRITICA ######################
                    //rilascio il semaforo dello studente invitato
                    release_sem(sem_id,student_invited);

                    array_invites[student_invited]=WAIT_ANSW;
                    my_invitations++;
                }
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
                    waiting_answers(array_invites)){
                    message.mtype = sender+100;
                    sprintf(message.text,"%d denied",this_student->matricola);
                    msgsnd(msg_id,&message,sizeof(message.text),0);
                    TEST_ERROR;
                //valuta la risposta
                } else {
                    //se e' conveniente accettare e il numero di rifiuti e' minore del massimo
                    if(accept_invite(shared,sender) && my_rejects<max_reject){
                        message.mtype = sender+100;
                        sprintf(message.text,"%d accepted",this_student->matricola);
                        msgsnd(msg_id,&message,sizeof(message.text),0);
                        TEST_ERROR;
                        state = MEMBER;
                    } else { //se rifiuta
                        message.mtype = sender+100;
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
                    struct info_group *this_group;
                    this_student->group = this_student->matricola;
                    this_group = &shared->group[this_student->group];
                    
                    reserve_sem(sem_id,SEM_SHM);
                    // ############### INIZIO SEZIONE CRITICA ####################
                    this_group->number++;
                    //se il voto di quello che ha accettato e' maggiore del massimo voto del gruppo
                    if(this_group->max_voto<shared->student[sender].voto_AdE)
                        this_group->max_voto = shared->student[sender].voto_AdE;
                    //se ha raggiunto il numero di membri preferiti chiudi il gruppo
                    if(shared->group[this_student->group].number == this_student->nof_elems)
                        shared->group[this_student->group].is_closed = TRUE;
                    // ############### FINE SEZIONE CRITICA ######################
                    release_sem(sem_id,SEM_SHM);
                    
                    array_invites[sender]=ANSWERED;
                    state = LEADER;
                //se e' stato rifiutato un invito
                } else if(strcmp(buf,"denied")==0)
                    array_invites[sender]=ANSWERED;
            }
        }
        //rilascia il semaforo con la sua matricola
        release_sem(sem_id,this_student->matricola);
    }
}