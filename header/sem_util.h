#ifndef _SEM_UTIL_H_
#define _SEM_UTIL_H_

#include <sys/types.h>
#include <sys/sem.h>

//definizione visuale dei semafori ready, per la memoria condivisa di info_student e gruppo
#define SEM_READY 0
#define SEM_SHM 1
#define N_SEM 2 //dal 2 in poi ci sono i semafori riservati agli studenti (quanti? POP_SIZE)

union semun {
    int val;                    // value for SETVAL
    struct semid_ds* buf;       // buffer for IPC_STAT, IPC_SET
    unsigned short* array;      // array for GETALL, SETALL
#if defined(__linux__)
    struct seminfo* __buf;      // buffer for IPC_INFO
#endif
};

// Ritorna il valore del semaforo (consideando anche valori negativi)
int get_sem_val(int semid, int semnum);

// Inizializzazione del semaforo a valore inserito da utente
int init_sem(int semid, int semnum, int semval);

// Inizializzazione del semaforo a 1 (i.e., "available")
int init_sem_available(int semid, int semnum);

// Inizializzazione del semaforo a 0 (i.e., "in use")
int init_sem_in_use(int semid, int semnum);

// Reserve semaphore - decremento di 1
int reserve_sem(int semid, int semnum);

// Release semaphore - incremento di 1
int release_sem(int semid, int semnum);


#endif
