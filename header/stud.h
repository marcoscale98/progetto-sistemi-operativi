#ifndef _STUD_H_
#define _STUD_H_

void accetta_invito(int mittente , int destinatario , int msg_id);

int chiudo_gruppo();

//controlla gli inviti ricevuti e li valuta
//return true se accetta un invito
//return false se non accetta
int rispondo_inviti(int msg_id, int *accettato);

//controlla se ha ricevuto risposta agli inviti
//return true se tutti hanno risposto
//return false se qualcuno non ha risposto
int controllo_risposte(int msg_id, int *invitati);

//controlla che tutti gli invitati abbiano risposto agli inviti
int hanno_risposto(int *invitati);

void inserisci_nel_mio_gruppo(int matricola);

void invita_studente(int *invitati, int mittente, int destinatario, int msg_id);

void mando_inviti(int msg_id, int *invitati) ;

void rifiuta_invito(int mittente, int destinatario, int msg_id);

//funzione che imposta a -1 l'elemento che contiene la matricola uguale a mittente
void setta_risposta(int *invitati,int mittente);

//confronta le matricole di 2 studenti e verifica siano nello stesso turno
int stesso_turno (struct info_student *mat1, struct info_student *mat2);

int max(int num1, int num2);

#endif
