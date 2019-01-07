#ifndef _SIG_UTIL_H_
#define _SIG_UTIL_H_

//handler per SIGINT: rimuove le strutture ipc e poi uccide il process group
void handler_sigint(int sig);

//handler per SIGALRM: allo scadere del timer sospende i processi studente
void handler_sigalrm(int sig);

//funzione richiamata per impostare l'handler di SIGINT
int sa_sigint();

//funzione richiamata per impostare l'handler di SIGSEGV
int sa_sigsegv();

//funzione richiamata per impostare l'handler di SIGALRM
int sa_sigalrm();

#endif
