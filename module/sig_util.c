#include "../header/error.h"
#include "../header/sem_util.h"
#include "../header/sig_util.h"
#include "../header/config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/signal.h>

//handler per SIGINT: rimuove le strutture ipc e poi uccide il process group
void handler_sigint(int sig){
    printf("Gestore (PID: %d). Ricevuto SIGINT: uccisione del process group\n",getpid());

    int sem_id, msg_id, shm_id;
    sem_id = semget(IPC_KEY,N_SEM,0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY,0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE,0666);
    TEST_ERROR;
    
    //rimozione ipc
    semctl(sem_id,IPC_RMID, NULL);
    TEST_ERROR;
    shmctl(shm_id,IPC_RMID, NULL);
    TEST_ERROR;
    msgctl(msg_id,IPC_RMID, NULL);
    TEST_ERROR;
    
    //uccisione processi
    killpg(0,SIGKILL);
    TEST_ERROR;
}

//handler per SIGALRM: allo scadere del timer sospende i processi studente
void handler_sigalrm(int sig){
    printf("Gestore (PID: %d). Tempo scaduto! Gli studenti si fermino\n",getpid());
    killpg(0,SIGSTOP);
    TEST_ERROR;
}

//funzione richiamata per impostare l'handler di SIGINT
int sa_sigint(){
    struct sigaction sa;
    sa.sa_handler = handler_sigint;
    return sigaction(SIGINT,&sa,NULL);
}

//funzione richiamata per impostare l'handler di SIGALRM
int sa_sigalrm(){
    struct sigaction sa;
    sa.sa_handler = handler_sigalrm;
    return sigaction(SIGALRM,&sa,NULL);
}
