#include <sys/types.h>
#include <sys/sem.h>
#include "../header/sem_util.h"

// Inizializzazione del semaforo a valore inserito da utente
int init_sem(int semid, int semnum, int semval) {
   union semun arg;
   arg.val = semval;
   return semctl(semid, semnum, SETVAL, arg);
}

// Inizializzazione del semaforo a 1 (i.e., "available")
int init_sem_available(int semid, int semnum) {
   union semun arg;
   arg.val = 1;
   return semctl(semid, semnum, SETVAL, arg);
}

// Inizializzazione del semaforo a 0 (i.e., "in use")
int init_sem_in_use(int semid, int semnum) {
   union semun arg;
   arg.val = 0;
   return semctl(semid, semnum, SETVAL, arg);
}

// Reserve semaphore - decremento di 1
int reserve_sem(int semid, int semnum) {
   struct sembuf sops;
   sops.sem_num = semnum;
   sops.sem_op = -1;
   sops.sem_flg = 0;
   return semop(semid, &sops, 1);
}

// Release semaphore - incremento di 1
int release_sem(int semid, int semnum) {
   struct sembuf sops;
   sops.sem_num = semnum;
   sops.sem_op = 1;
   sops.sem_flg = 0;
   return semop(semid, &sops, 1);
}
