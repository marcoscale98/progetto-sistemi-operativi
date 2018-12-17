#ifndef _SEM_UTIL_
#define _SEM_UTIL_

union semun {
    // value for SETVAL
    int val;
    // buffer for IPC_STAT, IPC_SET
    struct semid_ds* buf;
    // array for GETALL, SETALL
    unsigned short* array;
    // Linux specific part
   #if defined(__linux__)
    // buffer for IPC_INFO
    struct seminfo* __buf;
   #endif
};
// Inizializzazione semaforo a valore inserito da utente
int initSem(int semId, int semNum, int semVal);

// Inizializzazione del semaforo a 1 (i.e., "available")
int initSemAvailable(int semId, int semNum);

// Inizializzazione del semaforo a 1 0 (i.e., "in use")
int initSemInUse(int semId, int semNum);

// Reserve semaphore - decremento di 1
int reserveSem(int semId, int semNum);

// Release semaphore - incremento di 1
int releaseSem(int semId, int semNum);

#endif