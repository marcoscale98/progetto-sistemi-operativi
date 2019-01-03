#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "header/stud.h"

#define TRUE 1 
#define FALSE 0
#define ARRAY_LEN 100

//#define DEBUG

//funzioni
void invita_processo(int *invitati, int mittente, int destinatario, int msg_id,int i){
    struct msgbuf invito;

    invito.mytype = (long)destinatario;
    invito.text = "Invito : ";

    char buf[8];
    sprintf(buf,"%d",mittente);
    strcat(invito.text,buf);

    if(msgsnd(msg_id,&invito,MSG_LEN,0)==-1) {
        fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Invito Spedito");
    invitati[i]=destinatario;
    i++;
}

void rifiuta_invito(int *invitati, int mittente, int msg_id, int n_rifiuti,int max_reject){

    struct msgbuf invito;
    int destinatario;


    while(msgrcv(msg_id,&invito,MSGLEN,mittente,IPC_NOWAIT)!=1){
        if(n_rifiuti<=max_reject){
            sscanf(invito.text,"%s : %d", messaggio_invito[50] , &pid_invitatore)
            messaggio.type= pid_invitatore;
            messaggio.testo="Rifiuto : "
            strcat(messaggio.testo, arg[1]); //matricola di chi accetta
            if(msgsnd(id, &messaggio, MSG_LEN, 0)<0) {
                fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            TEST_ERROR
            printf("Invito Spedito");
            n_rifiuti ++;
        }else{
            accetta_invito()
        }
    }
}

void accetta_invito(){

    int pid_invitatore;
    char messaggio_invito[50];
    char accetta_messaggio[50];
    struct msgbuf accetta_invito;

    while(msgrcv(id,&invito,MSGLEN,stud->matricola,IPC_NOWAIT)!=-1){
        sscanf(invito.text,"%s : %d", messaggio_invito[50] , &pid_invitatore)
        accetta_invito.type= pid_invitatore;
        accetta_invito.testo="Accetto : "
        strcat(accetta_invito.testo, arg[1]); //matricola di chi accetta
        if(msgsnd(id, &accetta_invito, MSG_LEN, 0)<0) {
            fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        else
            printf("Invito Spedito");
    }
    invita_processo();
}    

//conttrolla che tutti gli invitati abbiano risposto agli inviti
int controlla_risposta(int *invitati){
    int return_value = TRUE;
    int i;
    for(i=0;i<ARRAY_LEN;i++){
        if(invitati[i]==-1){
            return_value = FALSE;
            break;
        }
    }
    return return_value;
}

//funzione che imposta a -1 l'elemento che contiene la matricola uguale a mittente
void setta_risposta(int *invitati,int mittente){
    int i=0;
    while(i<ARRAY_LEN && !invitati[i]==mittente){
        i++;
    }
    invitati[i]=-1;
}
// fine funzioni

//codice studente
int main(int argc,char *argv[]){ //argv[0]="student", argv[1]=matricola, argv[2]=prob_2, argv[3]=prob_3, \
    argv[4]=nof_invites, argv[5]=max_reject

    if(argc!=6){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }
    
    //INIZIALIZZAZIONE IPC
    int sem_id, shm_id, msg_id;
    sem_id = semget(IPC_KEY, N_SEM, 0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY, 0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE, 0666);
    TEST_ERROR;
    
    //INIZIALIZZAZIONE VARIABILI STUDENTE    
    struct info_student student;
    float prob_2 = atoi(argv[2])/100.0,
          prob_3 = atoi(argv[3])/100.0;
    int nof_invites = atoi(argv[4]),
        max_reject = atoi(argv[5]);
        
    student.matricola = atoi(argv[1]);
    student.group = NOGROUP; //non ancora in un gruppo
    //l'indice dei gruppi parte dall'1
    
    //inizializzazione voto_AdE
    srand(getpid());
    student.voto_AdE = rand()%13 + 18;  //compreso tra 18 e 30

    //inizializzazione nof_elems
    srand(getpid());
    int val = rand()%POP_SIZE;
    if(val<POP_SIZE*prob_2)
        student.nof_elems = 2;
    else if(val>=POP_SIZE*prob_2 && val<POP_SIZE*(prob_2+prob_3))
        student.nof_elems = 3;
    else //if(val>=popsize*(prob_2+prob_3) && val<popsize)
        student.nof_elems = 4;
     
    
    //INIZIALIZZAZIONE VARIABILI MIO GRUPPO (per ora ci sono solo io)
    struct info_group my_group;
    my_group.n_members=1;
    my_group.is_closed=FALSE;
    my_group.pref_nof_elems=student.nof_elems;
    my_group.max_voto=student.voto_AdE;
    
    //INIZIALIZZAZIONE MEMORIA CONDIVISA
    struct info_sim *aula = (struct info_sim *)shmat(shm_id, NULL, 0666);
    aula->student[student.matricola] = student;
    aula->group[student.matricola] = my_group;
    //non c'è bisogno di un semaforo perchè non c'è sovrapposizione delle aree interessate
    
#ifdef DEBUG
    printf(" student.matricola: %d\n", student.matricola);
    printf(" prob_2: %f\n", prob_2);
    printf(" prob_3: %f\n", prob_3);
    printf(" nof_invites: %d\n", nof_invites);
    printf(" max_reject: %d\n", max_reject);
    printf(" student.group: %d\n", student.group);
    printf(" student.voto_AdE: %d\n", student.voto_AdE);
    printf(" student.nof_elems: %d\n", student.nof_elems);
#endif
        
    //studenti invitati con successo
    int invitati[ARRAY_LEN];
    int i = 0;  //indice di invitati

    int counter_invites = 0;    //per controllare nof_invites
    int n_rifiuti=0;    //per controllare max_rejects
    
    //fine inizializzazione
    reserve_sem(sem_id, SEM_READY);
    TEST_ERROR;

    //inizio formazione dei gruppi
    while(counter_invites < nof_invites && (student.group == NOGROUP || aula->group[student.group].n_members < student.nof_elems)){

        struct msgbuf risposta;
    	char messaggio[50];
    	int mittente;

        //controlla se ha messaggi
    	while(msgrcv(msg_id,&risposta,MSGLEN,student.matricola,IPC_NOWAIT)!=-1){
    		sscanf(risposta.text,"%s : %d", messaggio[50], &mittente);
    		if(strcmp("Accetto",messaggio)==0 || strcmp("Rifiuto",messaggio)==0){
    			setta_risposta(invitati,mittente);
    		}
    		else{ //ricezione altro invito 
    			if(controlla_risposta(invitati))
    				//Scegliere Processo da Invitare per passarlo alla funzione di invito
    				invita_processo(invitati);
    			else
    				rifiuta_invito();
    		}
    	}
    	if(student.group!=NOGROUP){
    		rifiuta_invito();
    	else{
    		invito_processo();
    		//inserire controllo risposta e il messaggio ricevuto è accetta , diventa leader del gruppo 
  			counter_invites ++;
  		}
  	}
    return 0;          
}
