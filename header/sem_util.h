#ifndef _SEM_UTIL_H_
#define _SEM_UTIL_H_

#include <sys/types.h>
#include <sys/sem.h>
#include "config.h"

//definizione visuale dei semafori ready, per la memoria condivisa di info_student e gruppo
//i semafori precedenti sono riservati agli studenti

//i seguenti due semafori servono a sincronizzare studenti e gestori durante l'inizializzazione
#define SEM_READY POP_SIZE
#define SEM_GO (1+POP_SIZE)

//semaforo di scrittura nei campi group
#define WRITE_GROUP (2+POP_SIZE)
//semaforo di mutua esclusione per la modifica di lettori_group
#define MUTEX_GROUP (3+POP_SIZE)

//semaforo di scrittura nel campo time
#define WRITE_TIME (4+POP_SIZE)
//semaforo di mutua esclusione per la modifica di lettori_time
#define MUTEX_TIME (5+POP_SIZE)

//numero totale di semafori
#define N_SEM (6+POP_SIZE)

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

// Ritorna il valore del semaforo
int get_sem_val2(int semid, int semnum);

// Inizializzazione del semaforo a valore inserito da utente
int init_sem(int semid, int semnum, int semval);

// Inizializzazione del semaforo a 1 (i.e., "available")
int init_sem_available(int semid, int semnum);

// Inizializzazione del semaforo a 0 (i.e., "in use")
int init_sem_in_use(int semid, int semnum);

// Reserve semaphore - decremento di 1
int reserve_sem(int semid, int semnum);

// Reserve semaphore - decremento di 1
int reserve_sem_nowait(int semid, int semnum);

// Release semaphore - incremento di 1
int release_sem(int semid, int semnum);

// Test semaphore if is zero
int test_sem_zero(int semid, int semnum);

#endif
