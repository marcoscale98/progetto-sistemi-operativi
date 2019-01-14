#ifndef _STUD_H_
#define _STUD_H_

#include "shm_util.h"

#define MSG_LEN 100
//etichette per definire gli stati degli inviti
#define LIBERO 2
#define DA_INVITARE 1
#define INVITATO 0
#define RISPOSTO -1

struct msgbuf{
   long mtype;             /* message type, sarà > 0 */
   char text[MSG_LEN];    /* message testo */
};

//controlla se ha ricevuto risposta agli inviti
//return true se tutti hanno risposto
//return false se qualcuno non ha risposto
int controllo_risposte(int *is_leader, int *invitati, int n_invitati, int *inviti);

//controlla gli inviti ricevuti e li valuta
//return true se accetta un invito
//return false se non accetta
int rispondo_inviti(int *accettato, int *is_leader,int *n_rifiutati, int max_reject, int *inviti);

int max(int num1, int num2);

void algoritmo_inviti(int *invitati, int *n_invitati, int nof_invites);

//confronta le matricole di 2 studenti e verifica siano nello stesso turno
int stesso_turno (struct info_student *mat1, struct info_student *mat2);

//decido se conviene chiudere il gruppo(return true) oppure no(return false)
int chiudo_gruppo(int *is_leader);

void inserisci_nel_mio_gruppo(int matricola, int *is_leader);

void invita_studente(int destinatario);

void rifiuta_invito(int mittente, int *n_rifiutati);

void accetta_invito(int mittente);

//controlla che tutti gli invitati abbiano risposto agli inviti
int hanno_risposto(int *invitati);

//handler per SIGUSR1: per processi studente
void handler_sigusr1(int sig);

//funzione richiamata per impostare l'handler di SIGALRM
int sa_sigusr1();

#endif
