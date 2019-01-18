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


int write_log(struct sim_opt opt, int voto_AdE, int voto_SO){
    FILE *fd = fopen("logfile.log","a");
    if(fd){
        fprintf(fd, "\n%s, %s\n", __DATE__, __TIME__);
        fprintf(fd, "<%d, %d, %d, %d, %d, %d, %d, %d, %d>\n",
                POP_SIZE, opt.prob_2, opt.prob_3, opt.prob_4, opt.nof_invites,
                opt.max_reject, opt.sim_time,voto_AdE, voto_SO);
        fclose(fd);
        return 0;
    }
    return -1;    
}

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
int print_data(int array[], int size){
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
        printf("VOTO MEDIO = %d\n\n",sum/POP_SIZE);
        return sum/POP_SIZE;
    }
    return -1;
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

    if(POP_SIZE<=1){
        printf("Numero di Studenti invalido, inserire un numero maggiore di 1\n");
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
    
    //inizializzazione dei semafori
    init_sem_available(sem_id,MUTEX);
    TEST_ERROR;
    init_sem_available(sem_id,WRITE);
    TEST_ERROR;
    //semaforo SEM_GO inizializzato a 0: per bloccare gli studenti dopo la loro inizializzazione
    init_sem_in_use(sem_id,SEM_GO);
    TEST_ERROR;
    //semaforo SEM_READY inizializzato a POPSIZE: per avvisare il gestore che tutti gli studenti sono pronti al via!
    init_sem(sem_id, SEM_READY, POP_SIZE);
    TEST_ERROR;
    init_sem(sem_id, MSG_QUEUE, 50);
    TEST_ERROR;

    //inizializzazione memoria condivisa
    shared->time_left = options.sim_time;
    shared->lettori = 0;

#ifdef DEBUG
    printf("Gestore (PID: %d). Create IPCS e inizializzate\n",getpid());
#endif
    //creazione dei figli
    int value, i; 
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
                TEST_ERROR;
                break;
        }    
    }
#ifdef DEBUG
    printf("_Gestore (PID: %d). Creati figli\n",getpid());
#endif
    //attesa dell'inizializzazione degli studenti
    //while(get_sem_val(sem_id,SEM_READY)!=-POP_SIZE);
    test_sem_zero(sem_id, SEM_READY);
    
    //set del timer e inizio simulazione
    time_t start=time(NULL), timer;
    TEST_ERROR;
    printf("Gestore (PID: %d). Timer inizializzato e inizio simulazione\n",getpid());

    //sblocco degli studenti
    init_sem(sem_id,SEM_GO,POP_SIZE);
    
#ifdef DEBUG
    printf("_Gestore (PID: %d). Studenti sbloccati\n",getpid());
#endif
    int k=1;
    while((int)(time(&timer)-start) < options.sim_time) {
        //il timer si aggiorna ogni 5% di sim_time
        if((int)(timer-start) == (int)(options.sim_time*(0.05*k))) {
           
            shared->time_left = options.sim_time - (int)(timer-start); //tempo rimanente
            //#ifdef DEBUG
            printf("Gestore (PID: %d): Tempo rimanente = %2d secondi.\n", getpid(), shared->time_left);
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
    while(msgrcv(msg_id,&message,sizeof(message.text),(long)0,IPC_NOWAIT)!=-1);
    errno=0; //perchè la IPC_NOWAIT genera un errore se non trova nulla
    
#ifdef DEBUG
    printf("_Gestore (PID: %d). Calcolo dei voti\n",getpid());
#endif
    //CALCOLO DEI VOTI
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
    while(waitpid(-1, NULL, 0)!=-1);
    errno=0;    //inserito solo per non far visualizzare l'errore della waitpid (è normale che dia errore)

    #ifdef DEBUG
    printf("\n#####################################################################\n"\
            "STATO FINALE DEI GRUPPI\n\n");
    for(i=0;i<POP_SIZE;i++){
        if(shared->group[i].n_members!=0){
            printf("Informazioni gruppo n°%d, is_closed: %d\n", i, shared->group[i].is_closed);
            int j;
            for(j=0;j<POP_SIZE;j++){
                if(shared->student[j].group==i)
                    printf("- matricola: %d, nof_elems: %d, voto_AdE: %d\n",shared->student[j].matricola,shared->student[j].nof_elems,
                        shared->student[j].voto_AdE);
            }
            printf("\n");
        }
    }
    #endif

    //stampa dei dati della simulazione
    int m_AdE, m_SO;
    printf("\n#####################################################################\n");
    printf("Gestore (PID: %d). Dati dei voti di Architettura degli Elaboratori:\n",getpid());
    m_AdE = print_data(AdE,POP_SIZE);
    printf("#####################################################################\n");
    printf("Gestore (PID: %d). Dati dei voti di Sistemi Operativi:\n",getpid());
    m_SO = print_data(SO,POP_SIZE);
    write_log(options,m_AdE,m_SO);

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
