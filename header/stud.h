#ifndef _STUD_H_
#define _STUD_H_

#include "shm_util.h"

#define MSG_LEN 100

struct msgbuf{
   long mtype;             /* message type, sarà > 0 */
   char text[MSG_LEN];    /* message testo */
};

//handler per SIGUSR1: per processi studente
void handler_sigusr1(int sig);

//funzione richiamata per impostare l'handler di SIGALRM
int sa_sigusr1();

#endif
