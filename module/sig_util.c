#include "../header/error.h"
#include "../header/sem_util.h"
#include "../header/sig_util.h"
#include "../header/shm_util.h"
#include "../header/stud.h"
#include "../header/config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>


//handler per SIGINT e SIGSEGV: rimuove le strutture ipc e poi uccide il process group
void handler_sigint(int sig){
    printf("Gestore (PID: %d). Ricevuto %s: uccisione del process group\n",getpid(),strsignal(sig));

    int sem_id, msg_id, shm_id;
    sem_id = semget(IPC_KEY,N_SEM,0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY,0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE,0666);
    TEST_ERROR;
    
    //rimozione ipc
    semctl(sem_id,0,IPC_RMID);
    TEST_ERROR;
    shmctl(shm_id,IPC_RMID,NULL);
    TEST_ERROR;
    msgctl(msg_id,IPC_RMID,NULL);
    TEST_ERROR;
    
    //uccisione processi
    killpg(0,SIGKILL);
    TEST_ERROR;
}

//handler per SIGALRM: allo scadere del timer sospende i processi studente
void handler_sigalrm(int sig){
    printf("Gestore (PID: %d). Tempo scaduto! Gli studenti si fermino\n",getpid());

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;    //fa ignorare al gestore il segnale sigusr1
    sigaction(SIGUSR1,&sa,NULL);
    TEST_ERROR;
    killpg(0,SIGUSR1);
    TEST_ERROR;
}

//handler per SIGUSR1: per processi studente
void handler_sigusr1(int sig){
    printf("Student (PID: %d). Bloccato! Aspetto il voto\n",getpid());
}

//funzione richiamata per impostare l'handler di SIGUSR1
int sa_sigusr1(){
    struct sigaction sa;
    sa.sa_handler = handler_sigusr1;
    return sigaction(SIGUSR1,&sa,NULL);
}

//funzione richiamata per impostare l'handler di SIGINT
int sa_sigint(){
    struct sigaction sa;
    sa.sa_handler = handler_sigint;
    return sigaction(SIGINT,&sa,NULL);
}

//funzione richiamata per impostare l'handler di SIGSEGV
int sa_sigsegv(){
    struct sigaction sa;
    sa.sa_handler = handler_sigint;
    return sigaction(SIGSEGV,&sa,NULL);
}

//funzione richiamata per impostare l'handler di SIGALRM
int sa_sigalrm(){
    struct sigaction sa;
    sa.sa_handler = handler_sigalrm;
    return sigaction(SIGALRM,&sa,NULL);
}


/*
//
//  PROVE (NON FUNZIONANO)
//

//handler per il gestore: distingue i vari segnali
void handler_gestore(int sig){
    int sem_id, msg_id, shm_id;
    sem_id = semget(IPC_KEY,N_SEM,0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY,0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE,0666);
    TEST_ERROR;

    struct sigaction sa;
    sigset_t mask;
    printf("Gestore (PID: %d). %s\n",getpid(),strsignal(sig));
    switch(sig){
        case SIGINT:
        case SIGSEGV:
            printf("Gestore (PID: %d). Uccisione del process group\n",getpid());
            //rimozione ipc e uccisione processi
            semctl(sem_id,0,IPC_RMID);
            shmctl(shm_id,IPC_RMID,NULL);
            msgctl(msg_id,IPC_RMID,NULL);
            TEST_ERROR;
            killpg(0,SIGKILL);
            break;
        case SIGALRM:
            printf("Gestore (PID: %d). Tempo scaduto! Gli studenti si fermino\n",getpid());
            //fa ignorare al gestore il segnale sigusr1
            sa.sa_handler = SIG_IGN;
            sigaction(SIGUSR1,&sa,NULL);
            TEST_ERROR;
            killpg(0,SIGUSR1);
            TEST_ERROR;
            //reimposta gli handler
            sa.sa_handler = handler_gestore;
            sigaction(SIGALRM,&sa,NULL);
            TEST_ERROR;
            sigemptyset(&mask);
            sigaddset(&sa.sa_mask,SIGINT);
            sigaddset(&sa.sa_mask,SIGSEGV);
            TEST_ERROR; 
            sigprocmask(SIG_BLOCK,&mask,NULL);
            TEST_ERROR;
            break;
    }
}

//funzione richiamata per impostare gli handler del gestore
int sa_gestore(){
    struct sigaction sa;
    sigset_t mask;

    sa.sa_handler = handler_gestore;
    sigaction(SIGINT,&sa,NULL);
    TEST_ERROR;
    sigemptyset(&mask);
    sigaddset(&sa.sa_mask,SIGALRM);
    sigaddset(&sa.sa_mask,SIGINT);
    sigaddset(&sa.sa_mask,SIGSEGV);
    TEST_ERROR;
    return sigprocmask(SIG_BLOCK,&mask,NULL);
}

//handler per il student: distingue i vari segnali
void handler_student(int sig){
    struct sigaction sa;
    printf("Student (PID: %d). %s\n",getpid(),strsignal(sig));
    switch(sig){
        case SIGSEGV:
            printf("Student (PID: %d). Termino\n",getpid());
            break;
        case SIGUSR1:
            printf("Student (PID: %d). Bloccato! Aspetto il voto\n",getpid());;
            //reimposta gli handler
            sa.sa_handler = handler_student;
            sigaction(SIGSEGV,&sa,NULL);
            TEST_ERROR;
            break;
    }
}

//funzione richiamata per impostare gli handler del gestore
int sa_student(){
    struct sigaction sa;
    sigset_t mask;

    sa.sa_handler = handler_student;
    sigaction(SIGUSR1,&sa,NULL);
    TEST_ERROR;
    sigemptyset(&mask);
    sigaddset(&sa.sa_mask,SIGUSR1);
    sigaddset(&sa.sa_mask,SIGSEGV);
    TEST_ERROR;
    return sigprocmask(SIG_BLOCK,&mask,NULL);
}
*/
