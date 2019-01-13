#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "header/error.h"
#include "header/conf_reader.h"
#include "header/sem_util.h"
#include "header/sig_util.h"
#include "header/shm_util.h"
#include "header/time_util.h"
#include "header/config.h"
#include "header/stud.h"


void print_array(int *a, int sz){
    if(a && sz>0){
        printf("Stampa array\n");
        int i;
        for(i=0;i<sz;i++){
            printf("Student[%d]= %d\n",i,a[i]);
        }
    }
}

// stampa per ogni voto il numero di studenti che ha tale voto
void print_data(int array[], int size){
    if(array && size>0){
        //print_array(array,size);
        printf("VOTO\tFREQUENZA\n");

        int v, i,cnt, sum=0;
        for(v=0;v<=30;v++){
            for(i=0, cnt=0;i<size;i++){
                if(v==array[i])
                    cnt++;
            }
            if(cnt>0){
                printf("%d\t%d\n",v,cnt);
                sum += cnt*v;
            }
        }
        printf("VOTO MEDIO = %d\n",sum/POP_SIZE);
    }
}

int main(){                 //codice del gestore
    //set degli handler
    sa_sigsegv();
    TEST_ERROR;
    sa_sigint();
    TEST_ERROR;
    sa_sigalrm();
    TEST_ERROR;

    printf("Gestore (PID: %d). Inizio programma\n",getpid());

    //inizializzazione delle variabili della simulazione
    struct sim_opt options;
    if(init_options(&options)==-1){
        printf("ERRORE: PID= %d. %s, %d. Errore nell'inizializzazione delle variabili\n", getpid(), __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    //set di args
    char matricola[ARG_SIZE],
        prob_2[ARG_SIZE],
        prob_3[ARG_SIZE],
        nof_invites[ARG_SIZE],
        max_reject[ARG_SIZE];
    sprintf(prob_2,"%d",options.prob_2);
    sprintf(prob_3,"%d",options.prob_3);
    sprintf(nof_invites,"%d",options.nof_invites);
    sprintf(max_reject,"%d",options.max_reject);
    char *args[] = {"student",matricola,prob_2,prob_3,nof_invites,max_reject,NULL};

    //creazione ipc
    int sem_id, shm_id, msg_id;
    sem_id = semget(IPC_KEY,N_SEM,IPC_CREAT|IPC_EXCL|0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY, IPC_CREAT|IPC_EXCL|0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE,IPC_CREAT|IPC_EXCL|0666);
    TEST_ERROR;
    //acquisizione indirizzo memoria condivisa
    struct info_sim *shared;
    shared = shmat(shm_id,NULL,0);
    TEST_ERROR;
    if(sem_id==-1 || msg_id==-1 || shm_id==-1 ||shared==(void *)-1){
        printf("Gestore (PID: %d). Errore nella creazione delle IPCS\n",getpid());
        exit(EXIT_FAILURE);
    }
    
    //inizializzazione del semaforo di scrittura
    init_sem_available(sem_id,SEM_SHM);
    TEST_ERROR;
    //semaforo SEM_READY inizializzato a 0: per bloccare gli studenti dopo la loro inizializzazione
    init_sem_in_use(sem_id,SEM_READY);
    TEST_ERROR;

    printf("Gestore (PID: %d). Create IPCS e inizializzate\n",getpid());

    //creazione dei figli
    int i, value; 
    for(i=0, value=-1;value && i<POP_SIZE;i++){
        switch(value = fork()){
            case -1:
                TEST_ERROR;
                break;  
            case 0:
                sprintf(matricola,"%d",i);
                execvp("./student",args);
                printf("ERRORE: PID= %d, %s, %d. Invocazione di execvp fallita\n",getpid(),__FILE__,__LINE__);
                break;
            default:
                break;
        }    
    }
    printf("Gestore (PID: %d). Creati figli\n",getpid());

    //attesa dell'inizializzazione degli studenti
    while(get_sem_val(sem_id,SEM_READY)!=-POP_SIZE);

    //set del timer e inizio simulazione
    shared->time_left = options.sim_time;
    set_timer(options.sim_time);
    printf("Gestore (PID: %d). Timer inizializzato e inizio simulazione\n",getpid());

    //sblocco degli studenti
    init_sem(sem_id,SEM_READY,POP_SIZE);
#ifdef DEBUG
    printf("_Gestore (PID: %d). Studenti sbloccati\n",getpid());
#endif
/*********************************************************************************
 * 
 * SE STA FACENDO LA TIME_LEFT E SCATTA IL TIMER NON VIENE INVIATO IL SEGNALE AGLI STUDENTI
 * 
 * **********************************************************************************/
    //ciclo di aggiornamento time_left
    while(shared->time_left>0){
        shared->time_left = time_left();
    //#ifdef DEBUG
        printf("_Gestore (PID: %d): Tempo rimanente = %d secondi.\n", getpid(), shared->time_left);
    //#endif
        sleep(UPDATE_TIME);
    } //allo scattare del timer verrà invocato l'handler
    shared->time_left = 0;

    //pulizia della coda dei messaggi
    struct msgbuf message;
    while(msgrcv(msg_id,&message,sizeof(message.text),(long)0,IPC_NOWAIT)!=-1);
    errno=0; //perchè la IPC_NOWAIT genera un errore se non trova nulla
    
#ifdef DEBUG
    printf("_Gestore (PID: %d). Calcolo dei voti\n",getpid());
#endif
    //calcolo dei voti
    int AdE[POP_SIZE], SO[POP_SIZE];
    
    for(i=0;i<POP_SIZE;i++){
        //contiene la struttura dello studente in posizione i
        struct info_student stud = shared->student[i];
        //contiene il gruppo dello studente stud
        struct info_group grp;
        if(stud.group != NOGROUP)
            grp = shared->group[stud.group];

        //massimo voto che lo studente i puo' prendere
        int voto_SO = grp.max_voto;

        //azzera il voto se il gruppo non e' chiuso
        if(stud.group==NOGROUP || !grp.is_closed)
            voto_SO = 0;
        //sottrae 3 se non e' stata rispettata la preferenza
        else if(stud.nof_elems != grp.n_members)
            voto_SO-=3;

        //aggiornamento dei dati
        AdE[i]=stud.voto_AdE;
        SO[i]=voto_SO;

        //invio del messaggio allo studente con il suo voto
        message.mtype = (long)(stud.matricola+100); //+100 per evitare matricola=0
        sprintf(message.text,"%d",voto_SO);
        msgsnd(msg_id,&message,sizeof(message.text),0);
        TEST_ERROR;
    }

    //aspetto che gli studenti abbiano ricevuto e stampato i voti
    waitpid(-1, NULL, 0);

    //stampa dei dati della simulazione
    printf("Gestore (PID: %d). Dati dei voti di Architettura degli Elaboratori\n",getpid());
    print_data(AdE,POP_SIZE);
    printf("Gestore (PID: %d). Dati dei voti di Sistemi Operativi\n",getpid());
    print_data(SO,POP_SIZE);

    //detach memoria condivisa
    shmdt(shared);
    TEST_ERROR;

    //rimozione ipc
    semctl(sem_id, 0, IPC_RMID);
    TEST_ERROR;
    shmctl(shm_id,IPC_RMID,NULL);
    TEST_ERROR;
    msgctl(msg_id,IPC_RMID,NULL);
    TEST_ERROR;
} 
