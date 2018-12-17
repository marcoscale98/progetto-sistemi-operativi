#include <sys/types.h>
#include <sys/sem.h>
#include "header/sem_util.h"

// Inizializzazione del semaforo a valore inserito da utente
int initSem(int semId, int semNum, int semVal) {
   union semun arg;
   arg.val = semVal;
   return semctl(semId, semNum, SETVAL, arg);
}


// Inizializzazione del semaforo a 1 (i.e., "available")
int initSemAvailable(int semId, int semNum) {
   union semun arg;
   arg.val = 1;
   return semctl(semId, semNum, SETVAL, arg);
}

// Inizializzazione del semaforo a 0 (i.e., "in use")
int initSemInUse(int semId, int semNum) {
   union semun arg;
   arg.val = 0;
   return semctl(semId, semNum, SETVAL, arg);
}

// Reserve semaphore - decremento di 1
int reserveSem(int semId, int semNum) {
   struct sembuf sops;
   sops.sem_num = semNum;
   sops.sem_op = -1;
   sops.sem_flg = 0;
   return semop(semId, &sops, 1);
}

// Release semaphore - incremento di 1
int releaseSem(int semId, int semNum) {
   struct sembuf sops;
   sops.sem_num = semNum;
   sops.sem_op = 1;
   sops.sem_flg = 0;
   return semop(semId, &sops, 1);
}