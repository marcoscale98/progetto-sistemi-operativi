#ifndef SEM_UTIL
#define SEM_UTIL

#include <sys/types.h>
#include <sys/sem.h>

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

// Initialize semaphore to 1 (i.e., "available")
int initSemAvailable(int semId, int semNum) {
   union semun arg;
   arg.val = 1;
   return semctl(semId, semNum, SETVAL, arg);
}
// Initialize semaphore to 0 (i.e., "in use")
int initSemInUse(int semId, int semNum) {
   union semun arg;
   arg.val = 0;
   return semctl(semId, semNum, SETVAL, arg);
}

// Reserve semaphore - decrement it by 1
int reserveSem(int semId, int semNum) {
   struct sembuf sops;
   sops.sem_num = semNum;
   sops.sem_op = -1;
   sops.sem_flg = 0;
   return semop(semId, &sops, 1);
}
// Release semaphore - increment it by 1
int releaseSem(int semId, int semNum) {
   struct sembuf sops;
   sops.sem_num = semNum;
   sops.sem_op = 1;
   sops.sem_flg = 0;
   return semop(semId, &sops, 1);
}