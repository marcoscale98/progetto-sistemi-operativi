#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
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
        int i;
        for(i=0;i<POP_SIZE;i++){
            if(array[i]==WAIT_ANSW)
                return TRUE;
        }
        return FALSE;
    } else
    return -1;
}

int test_reset_invites(int *array, int my_matricola){
    if(array){
        int i;
        for(i=0;i<POP_SIZE;i++){
            if(my_matricola%2==i%2 && array[i]!=ANSWERED)
                return FALSE;
        }
        for(i=0;i<POP_SIZE;i++){
            array[i]=AVAILABLE;
        }
        return TRUE;
    } else
    return -1;
}

//funzione che ritorna lo studente da invitare nel caso di successo, -1 altrimenti
int choose_student(struct info_sim* shm, int* array, int my_matricola){
    if(shm && array && my_matricola>=0){
        struct info_student this_student = shm->student[my_matricola];
        int voto, i, student_invited;
        test_reset_invites(array, my_matricola);
        //se il voto dello studente e' compreso tra 27 e 30
        if(this_student.voto_AdE>=27){
            //scorro i voti dal piu' basso
            for(voto=18;voto<=30;voto++){
                //scorre gli studenti
                for(i=0;i<POP_SIZE;i++){
                    //lo studente e' dello stesso turno e ha lo stesso nof_elems
                    // e non sto aspettando risposta da lui e non appartiene a nessun gruppo
                    //
                    // shm->student[i].nof_elems==this_student.nof_elems &&
                    if(shm->student[i].voto_AdE==voto && i!=my_matricola && i%2==my_matricola%2 &&
                       array[i]==AVAILABLE &&
                       shm->student[i].group==NOGROUP)  //CRITICO!
                        student_invited = i;
                }
            }
        //se il voto dello studente e' compreso tra 18 e 26
        } else {
            //scorro i voti dal piu' alto
            for(voto=30;voto>=18;voto--){
                //scorre gli studenti
                for(i=0;i<POP_SIZE;i++){
                    //selo studente e' dello stesso turno
                    // e lo studente ha lo stesso nof_elems e non l'ho appena invitato
                    if(shm->student[i].voto_AdE==voto && i!=my_matricola && i%2==my_matricola%2 &&
                       array[i]==AVAILABLE && shm->student[i].group==NOGROUP)   //CRITICO!
                        student_invited = i;
                }
            }
        }
        return student_invited;
    //in caso di errore ritorna -1
    } else
    return -1;
}

//funzione che restituisce TRUE se lo studente accetta il voto e FALSE altrimenti
// modifica il campo group se lo studente ha accettato
int accept_invite(struct info_sim *shm, int sender,int my_matricola){
    if(shm && sender>=0 && my_matricola>=0){
        struct info_student *this_student = &shm->student[my_matricola],
                            inviter = shm->student[sender];
        //se il voto dello studente e' compreso tra 27 e 30
        if(this_student->voto_AdE>=27){
            //accetta se lo studente che lo ha invitato ha nof_elems uguale al suo
            if(inviter.nof_elems==this_student->nof_elems){
                this_student->group = inviter.group; //CRITIC0
                return TRUE;
            } else
                return FALSE;
        //se il voto dello studente e' compreso tra 18 e 26
        } else {
            //accetta se lo studente che lo ha invitato ha voto maggiore al suo
            if(inviter.voto_AdE>this_student->voto_AdE){
                this_student->group = inviter.group;    //CRITICO
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
    int nof_invites, max_reject, matricola;
    float prob_2, prob_3;
    matricola = atoi(argv[1]);
    prob_2 = atoi(argv[2])/100.0;
    prob_3 = atoi(argv[3])/100.0;
    nof_invites = atoi(argv[4]);
    max_reject = atoi(argv[5]);

    //INIZIALIZZAZIONE IPC
    int sem_id, shm_id, msg_id;
    sem_id = semget(IPC_KEY, N_SEM, 0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY, 0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE, 0666);
    TEST_ERROR;

    //INIZIALIZZAZIONE MEMORIA CONDIVISA
    struct info_sim* shared = (struct info_sim *)shmat(shm_id, NULL, 0666);
    TEST_ERROR;

    struct info_student *this_student = &shared->student[matricola];
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
    reserve_sem(sem_id,this_student->matricola);
    TEST_ERROR;

    // ######### INIZIO STRATEGIA #########
    int state = FREE,
        array_invites[POP_SIZE],
        my_invitations = 0,
        my_rejects = 0,
        i;
    for(i=0;i<POP_SIZE;i++)
        array_invites[i]=AVAILABLE;
    struct msgbuf message;

    //set tempo della simulazione
    int sim_time = shared->time_left;

    //finche' non scade il tempo
    while(shared->time_left > 0){
        //se manca poco e non sono un membro
        if(shared->time_left<=0.10*sim_time && state!=MEMBER){
            reserve_sem(sem_id,SEM_SHM);
            // ### SEZIONE CRITICA ###
            //se non sono ancora in un gruppo
            if(this_student->group==NOGROUP){
                this_student->group = this_student->matricola;
                shared->group[this_student->matricola].max_voto = this_student->voto_AdE;
                shared->group[this_student->matricola].number = 1;
            }
            shared->group[this_student->matricola].is_closed = TRUE;
            // ### FINE SEZIONE CRITICA ###
            release_sem(sem_id,SEM_SHM);
            #ifdef DEBUG
            printf("Il gruppo %d e' stato chiuso\n", this_student->group);
            #endif

        }

        // effettuando la reserve sul semaforo con la sua matricola
        // nessuno studente lo puo' invitare mentre sta controllando i messaggi
        reserve_sem(sem_id,this_student->matricola);
        //se non ha messaggi da leggere
        if(msgrcv(msg_id, &message, sizeof(message.text), this_student->matricola+100, IPC_NOWAIT)==-1){
            errno=0;
            TEST_ERROR;
            //rilascio il semaforo perche' ho finito di controllare
            release_sem(sem_id,this_student->matricola);
            //se non ha raggiunto il massimo di inviti 
            // e il suo gruppo non ha gia' raggiunto il numero di membri preferito
            // e non e' membro di un altro gruppo
            if(my_invitations < nof_invites &&
               (this_student->group == NOGROUP || shared->group[this_student->group].number < this_student->nof_elems) && //o e' chiuso
               state!=MEMBER){
                int student_invited = choose_student(shared,array_invites,this_student->matricola);
                #ifdef DEBUG
                printf("Lo studente %d ha scelto di invitare %d\n", this_student->matricola, student_invited);
                #endif
                message.mtype = student_invited+100;
                sprintf(message.text,"%d invite",this_student->matricola);
                
                //effettuo la reserve sul semaforo con la matricola dello studente che ho scelto
                reserve_sem(sem_id,student_invited);
                // ############### SEZIONE CRITICA ###########################
                msgsnd(msg_id,&message,MSG_LEN,0);
                TEST_ERROR;
                // ############### FINE SEZIONE CRITICA ######################
                //rilascio il semaforo dello studente invitato
                release_sem(sem_id,student_invited);

                #ifdef DEBUG
                printf("Lo studente %d ha invitato %d\nMessage: %s, type: %ld\n", this_student->matricola, student_invited,
                        message.text, message.mtype);
                #endif

                array_invites[student_invited]=WAIT_ANSW;
                my_invitations++;
            } //altrimenti non invita
        } else { //se ha messaggi da leggere
            //rilascio il semaforo perche' ho finito di controllare
            release_sem(sem_id,this_student->matricola);

            char buf[50];
            int sender;
            sscanf(message.text,"%d %s",&sender,buf);
            //se e' un invito
            if(strcmp(buf,"invite")==0){
                //se e' leader o membro o non aspetta risposte agli inviti
                if(state==LEADER || state==MEMBER || waiting_answers(array_invites)){
                    message.mtype = sender+100;
                    sprintf(message.text,"%d denied",this_student->matricola);
                    msgsnd(msg_id,&message,sizeof(message.text),0);
                    TEST_ERROR;
                    #ifdef DEBUG
                        printf("Lo studente %d e' stato costretto a rifiutare l'invito di %d\n", this_student->matricola, sender);
                        #endif
                //valuta la risposta
                } else {
                    //se e' conveniente accettare e il numero di rifiuti e' minore del massimo
                    if(accept_invite(shared,sender,this_student->matricola) && my_rejects>=max_reject){
                        message.mtype = sender+100;
                        sprintf(message.text,"%d accepted",this_student->matricola);
                        msgsnd(msg_id,&message,sizeof(message.text),0);
                        TEST_ERROR;
                        state = MEMBER;
                        #ifdef DEBUG
                        printf("Lo studente %d ha accettato l'invito di %d\n", this_student->matricola, sender);
                        #endif
                    } else { //se rifiuta
                        message.mtype = sender+100;
                        sprintf(message.text,"%d denied",this_student->matricola);
                        msgsnd(msg_id,&message,sizeof(message.text),0);
                        TEST_ERROR;
                        my_rejects++;
                        #ifdef DEBUG
                        printf("Lo studente %d ha rifiutato l'invito di %d\n", this_student->matricola, sender);
                        #endif
                    }
                }
            //se ha risposte agli inviti, non e' un membro di un gruppo
            // e il suo gruppo non e' chiuso
            } else if(state!=MEMBER &&
                      (this_student->group != NOGROUP || !shared->group[this_student->group].is_closed)){
                //se e' stato accettato un invito
                if(strcmp(buf,"accepted")==0){
                    struct info_group *this_group;
                    
                    reserve_sem(sem_id,SEM_SHM);
                    // ############### INIZIO SEZIONE CRITICA ####################
                    this_student->group = this_student->matricola;
                    this_group = &shared->group[this_student->group];
                    this_group->number++;
                    //se il voto di quello che ha accettato e' maggiore del massimo voto del gruppo lo aggiorna
                    if(this_group->max_voto<shared->student[sender].voto_AdE)
                        this_group->max_voto = shared->student[sender].voto_AdE;
                    //se ha raggiunto il numero di membri preferiti chiudi il gruppo
                    if(shared->group[this_student->group].number == this_student->nof_elems || shared->time_left<=0.10*sim_time){
                        shared->group[this_student->group].is_closed = TRUE;
                        #ifdef DEBUG
                        printf("Il gruppo %d e' stato chiuso\n", this_student->group);
                        #endif
                    }
                    // ############### FINE SEZIONE CRITICA ######################
                    #ifdef DEBUG
                    printf("Lo studente %d leader ha modificato il gruppo:\n", this_student->matricola);
                    int j;
                    for(j=0;j<POP_SIZE;j++){
                        if(shared->student[j].group==this_student->group)
                            printf("- matricola: %d, nof_elems: %d, voto_AdE: %d\n", j,
                                   shared->student[j].nof_elems,shared->student[j].voto_AdE);
                    }
                    printf("\n");
                    #endif
                    release_sem(sem_id,SEM_SHM);
                    
                    state = LEADER;
                } //se rifiuta non faccio nulla
                array_invites[sender]=ANSWERED;
            }
        }
    } //riceve il segnale e viene invocato l'handler
    //aspetta il voto dal gestore
    msgrcv(msg_id,&message,sizeof(message.text),this_student->matricola+100,0);
    printf("Student (PID: %d). voto_SO: %d\n", getpid(), atoi(message.text));

    //FINE SIMULAZIONE
    shmdt(shared);
    exit(EXIT_SUCCESS);
}