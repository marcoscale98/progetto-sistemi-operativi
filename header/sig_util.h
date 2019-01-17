#ifndef _SIG_UTIL_H_
#define _SIG_UTIL_H_

//handler per SIGINT: rimuove le strutture ipc e poi uccide il process group
void handler_sigint(int sig);

//funzione richiamata per impostare l'handler di SIGINT
int sa_sigint();

//funzione richiamata per impostare l'handler di SIGSEGV
int sa_sigsegv();

#endif
