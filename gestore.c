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
#include <time.h>
#include "header/error.h"
#include "header/conf_reader.h"
#include "header/sem_util.h"
#include "header/sig_util.h"
#include "header/shm_util.h"
#include "header/config.h"
#include "header/stud.h"

/*
void print_array(int *a, int sz){
    if(a && sz>0){
        printf("Stampa array\n");
        int i;
        for(i=0;i<sz;i++){
            printf("Student[%d]= %d\n",i,a[i]);
        }
    }
}*/

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
    sem_id = semget(IPC_KEY, N_SEM, IPC_CREAT|IPC_EXCL|0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY, IPC_CREAT|IPC_EXCL|0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE,IPC_CREAT|IPC_EXCL|0666);
    TEST_ERROR;
    //acquisizione indirizzo memoria condivisa
    struct info_sim *shared;
    shared = shmat(shm_id,NULL,0);
    TEST_ERROR
    if(sem_id==-1 || msg_id==-1 || shm_id==-1 ||shared==(void *)-1){
        printf("Gestore (PID: %d). Errore nella creazione delle IPCS\n",getpid());
        exit(EXIT_FAILURE);
    };
    
    //inizializzazione del semaforo di scrittura
    init_sem_available(sem_id,SEM_SHM);
    TEST_ERROR;
    //semaforo SEM_READY inizializzato a POP_SIZE
    init_sem(sem_id,SEM_READY,POP_SIZE);
    TEST_ERROR;

    #ifdef DEBUG
    printf("Gestore (PID: %d). Create IPCS e inizializzate\n",getpid());
    #endif

    //set del timer e inizio simulazione
    shared->time_left = options.sim_time;
    
    //creazione dei figli
    int value, i; 
    struct sembuf sops[POP_SIZE];
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
                //inizializzazione dei semafori degli studenti available
                init_sem_available(sem_id,i);
                sops[i].sem_num = i;
                sops[i].sem_op = 1;
                sops[i].sem_flg= 0;
                break;
        }    
    }

    #ifdef DEBUG
    printf("_Gestore (PID: %d). Creati figli\n",getpid());
    #endif
    
    //attesa dell'inizializzazione degli studenti
    struct sembuf ready;
    ready.sem_num = SEM_READY;
    ready.sem_op = 0;
    ready.sem_flg= 0;
    semop(sem_id,&ready,1);
    TEST_ERROR;

    //set del timer e inizio simulazione
    time_t start=time(NULL), timer;
    TEST_ERROR;
    printf("Gestore (PID: %d). Timer inizializzato e inizio simulazione\n",getpid());

    //sblocco degli studenti
    semop(sem_id,sops,POP_SIZE);
    TEST_ERROR;

    #ifdef DEBUG
    printf("_Gestore (PID: %d). Studenti sbloccati\n",getpid());
    #endif
    
    int k=1;
    while((int)(time(&timer)-start) < options.sim_time) {
        //il timer si aggiorna ogni 10% di sim_time
        if((int)(timer-start) == (int)(options.sim_time*(0.10*k))) {
            shared->time_left = options.sim_time - (int)(timer-start); //tempo rimanente
            //#ifdef DEBUG
            printf("_Gestore (PID: %d): Tempo rimanente = %d secondi.\n", getpid(), shared->time_left);
            //#endif
            k++;
        }
    }
    printf("Gestore (PID: %d). Tempo scaduto! Gli studenti si fermino\n",getpid());
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;    //fa ignorare al gestore il segnale sigusr1
    sigaction(SIGUSR1,&sa,NULL);
    TEST_ERROR;
    killpg(0,SIGUSR1);
    TEST_ERROR;
    shared->time_left=0;
    
    //pulizia della coda dei messaggi
    struct msgbuf message;
    while(msgrcv(msg_id,&message,sizeof(message.text),0,IPC_NOWAIT)!=-1);
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
        else if(stud.nof_elems != grp.number)
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
    while(waitpid(-1, NULL, 0)!=-1);
    errno=0;    //inserito solo per non far visualizzare l'errore

    //stampa dei dati della simulazione
    printf("#####################################################################\n");
    printf("Gestore (PID: %d). Dati dei voti di Architettura degli Elaboratori:\n",getpid());
    print_data(AdE,POP_SIZE);
    printf("#####################################################################\n");
    printf("Gestore (PID: %d). Dati dei voti di Sistemi Operativi:\n",getpid());

    //detach memoria condivisa
    shmdt(shared);
    TEST_ERROR;

    //rimozione ipc
    semctl(sem_id,0,IPC_RMID);
    TEST_ERROR;
    shmctl(shm_id,IPC_RMID,NULL);
    TEST_ERROR;
    msgctl(msg_id,IPC_RMID,NULL);
    TEST_ERROR;
} 
