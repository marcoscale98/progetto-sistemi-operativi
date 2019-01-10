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
#include "header/error.h"
#include "header/conf_reader.h"
#include "header/sem_util.h"
#include "header/sig_util.h"
#include "header/shm_util.h"
#include "header/time_util.h"
#include "header/config.h"
#include "header/stud.h"


// stampa per ogni voto il numero di studenti che ha tale voto
void print_data(int array[], int size){
    if(array && size>0){
        printf("VOTO\tFREQUENZA\n");

        int v, i,cnt;
        for(v=0;v<30;v++){
            for(i=0, cnt=0;i<size;i++){
                if(v==array[i])
                    cnt++;
            }
            if(cnt>0){
                printf("%d\t%d\n",v,cnt);
            }
        }
    }
}
//

int main(){                 //codice del gestore
    //set degli handler
    sa_sigint();
    TEST_ERROR;
    sa_sigalrm();
    TEST_ERROR;
    sa_sigsegv();
    TEST_ERROR;

#ifdef DEBUG
    printf("_Gestore (PID: %d). Inizio\n",getpid());
#endif
    //inizializzazione delle variabili della simulazione
    struct sim_opt options;
    if(init_options(&options)==-1){
        printf("ERRORE: PID: %d. %s, %d. Errore nell'inizializzazione delle variabili\n", getpid(), __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    printf("_Gestore (PID: %d). init_options fatta\n",getpid());
#endif

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

    if(sem_id==-1 || msg_id==-1 || shm_id==-1){
        printf("Gestore (PID: %d). Errore nella creazione delle IPCS\n",getpid());
        exit(EXIT_FAILURE);
    }

    //acquisizione indirizzo memoria condivisa
    struct info_sim *shared;
    shared = shmat(shm_id,NULL,0);
    TEST_ERROR;


    //inizializzazione del semaforo di scrittura
    init_sem_available(sem_id,SEM_SHM);
    TEST_ERROR;
    //semaforo SEM_READY inizializzato a 0
    init_sem_in_use(sem_id,SEM_READY);
    TEST_ERROR;

#ifdef DEBUG
    printf("_Gestore (PID: %d). Create ipc e inizializzate\n"\
           "Valore del SEM_SHM: %d\n"\
           "Valore del SEM_READY: %d\n",
           getpid(), get_sem_val(sem_id, SEM_SHM), get_sem_val(sem_id, SEM_READY));
#endif

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
                printf("ERRORE: PID: %d, %s, %d. Invocazione di execvp fallita\n",getpid(),__FILE__,__LINE__);
                break;
            default:
                /*
                #ifdef DEBUG
                printf("Gestore (PID: %d). Creato lo studente con matricola %d\n",getpid(),i);
                #endif
                */
                break;
        }    
    }
#ifdef DEBUG
    printf("_Gestore (PID: %d). Creati figli\n",getpid());
#endif

#ifdef DEBUG
    printf("_Gestore (PID: %d). Aspetto l'inizializzazione degli studenti\n",getpid());
#endif
    //attesa dell'inizializzazione degli studenti
    while(get_sem_val(sem_id,SEM_READY)!=-POP_SIZE);

    //set del timer e inizio simulazione
    set_timer(options.sim_time);
    shared->time_left = options.sim_time;
    printf("Gestore (PID: %d). Timer inizializzato e inizio simulazione\n",getpid());

    //sblocco degli studenti
    init_sem(sem_id,SEM_READY,POP_SIZE);
#ifdef DEBUG
    printf("Gestore (PID: %d). Studenti sbloccati\n",getpid());
#endif
    
    int timer;
    while((timer = time_left())>0){
        if(timer%5 == 0){       //aggiornamento del tempo rimanente ogni 5 secondi
            reserve_sem(sem_id,SEM_SHM);
            shared->time_left = timer;
            #ifdef DEBUG
                printf("Gestore (PID: %d): Tempo rimanente= %d secondi.\n", getpid(), timer);
            #endif
            release_sem(sem_id,SEM_SHM);
        }
    } //allo scattare del timer verrà invocato l'handler
    
#ifdef DEBUG
    printf("_Gestore (PID: %d). Calcolo dei voti\n",getpid());
#endif
    //calcolo dei voti
    int AdE[POP_SIZE], SO[POP_SIZE];
    memset(AdE,-1,sizeof(AdE));
    memset(SO,-1,sizeof(SO));

    for(i=0;i<POP_SIZE;i++){
 
        struct info_student stud = shared->student[i];      //contiene la struttura dello studente in posizione i
        struct info_group grp = shared->group[stud.group];      //contiene il gruppo dello studente stud
#ifdef DEBUG
        printf("indirizzo di grp: %p\n", &grp);
#endif
        int voto_SO = grp.max_voto;     //massimo voto che lo studente i puo' prendere

        if(!grp.is_closed) //azzera il voto se il gruppo non e' chiuso
            voto_SO = 0;
        else if(stud.nof_elems != grp.n_members)    //sottrae 3 se non e' stata rispettata la preferenza
            voto_SO-=3;

        //aggiornamento dei dati
        AdE[i]=stud.voto_AdE;
        SO[i]=voto_SO;
        
        //pulizia della coda dei messaggi

        //invio del messaggio allo studente con il suo voto
        struct msgbuf message;
        message.mtype = stud.matricola;
        sprintf(message.text,"%d",voto_SO);
        msgsnd(msg_id,&message,sizeof(message),0);
        TEST_ERROR;
    }

    //stampa dei dati della simulazione
    printf("Gestore (PID: %d). Dati dei voti di Architettura degli Elaboratori\n",getpid());
    print_data(AdE,POP_SIZE);
    printf("Gestore (PID: %d). Dati dei voti di Sistemi Operativi\n",getpid());
    print_data(SO,POP_SIZE);

    //rimozione ipc
    semctl(sem_id,0,IPC_RMID);      // *** non so perche' dia INTERRUPTED SYSTEM CALL ***
    TEST_ERROR;
    shmctl(shm_id,IPC_RMID,NULL);
    TEST_ERROR;
    msgctl(msg_id,IPC_RMID,NULL);
    TEST_ERROR;
} 
