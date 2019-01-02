#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "header/stud.h"

#define NOGROUP -1
#define MSG_LEN 100
#define TRUE 1 
#define FALSE 0
#define ARRAY_LEN 100

struct msgbuf{
	int mtype;             /* message type, sarà > 0 */
	char testo[MSG_LEN];    /* message testo */
};

int main(int argc,char *argv[]){ //argv[0]="student", argv[1]=matricola, argv[2]=prob_2, argv[3]=prob_3, \
    argv[4]=nof_invites, argv[5]=max_reject, argv[6]=POPSIZE , shmid = argv[7])
    
    if(argc!=8){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }

    int invitati[ARRAY_LEN];
    int i = 0;

    
    //definizione e inizializzazione variabili studente    
    struct info_student info_stud;
    float prob_2= atof(argv[2]),
        prob_3= atof(argv[3]);
    int nof_invites = atoi(argv[4]), //studen.nof_invites ?
    	max_reject = atoi(argv[5]),
    	POPSIZE = atoi(argv[6]),
    	shmid = atoi(argv[7]);
    
    info_stud.matricola = atoi(argv[1]);
    info_stud.group = NOGROUP; //non ancora in un gruppo
    //l'indice dei gruppi parte dall'1

    //inizializzazione nof_elems
    srand(getpid());
    int val = rand()%POPSIZE;
    if(val<POPSIZE*prob_2)
        info_stud.nof_elems = 2;
    else if(val>=POPSIZE*prob_2 && val<POPSIZE*(prob_2+prob_3))
        info_stud.nof_elems = 3;
    else //if(val>=POPSIZE*(prob_2+prob_3) && val<POPSIZE)
        info_stud.nof_elems = 4;

    if((msgget(ftok("opt.conf",0), SIRUSR | S_IWUSR)!-1); //utizzare ftok()
    	errExit("msgget")

    int counter_invites = 0;
    struct msgbuf risposta;
    int n_rifiuti=0;

    while(counter_invites < nof_invites & stud.group < stud.nof_elems){

    	char messaggio[50];
    	int pid_messaggio;

    	while((msgrcv(id,&risposta,MSGLEN,info_stud->matricola,IPC_NOWAIT)){
    		sscanf(risposta.text,"%s : %d", messaggio[50] , &pid_messaggio);
    		if(strcmp("Accetto",messaggio[50])==0 || strcmp("Rifiuto",messaggio[50])==0){
    			setta_risposta(invitati,pid_messaggio)
    		}
    		else{ //ricezione altro invito 
    			if(controlla_risposta)
    				//Scegliere Processo da Invitare per passarlo alla funzione di invito
    				invita_processo()
    			else
    				rifiuta_invito()

    		}
    	}

    	else if(info_student.group>-1){
    		rifiuta_invito();
    	else(){
    		invito_processo()
    		//inserire controllo risposta e il messaggio ricevuto è accetta , diventa leader del gruppo 
  			counter_invites ++;
  		}
  	}
  	
	
	void invito_processo(int type){

		char matricola[50] = argv[1];
		struct msgbuf invito;

		invito.mytype = (long)type
		invito.testo = "Vuoi_Paratecipare_al_Gruppo? : ";

		strcat(invito.testo,matricola);

		if(msgsnd(id, &invito, MSG_LEN, 0)<0) {
    			fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
    			exit(EXIT_FAILURE);
    	}

    	TEST_ERROR;
    	printf("Invito Spedito");
    	invitati[i]=type;
    	i++;
	}

	void rifiuta_invito(){

		struct msgbuf messaggio;
		int pid_invitatore;

		while(msgrcv(id,&invito,MSGLEN,stud->matricola,IPC_NOWAIT)!=1){
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
		struct msgbuf accetta_invito

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

	int controlla_risposta(int * invitati,int risposta){
		int return_value = FALSE;
		int j;
		for(j=0;j<ARRAY_LEN && invitati[j]==0 && !return_value ; j++)
			return_value = true;
		return return_value;
	}

	void setta_risposta(int * invitati,int risposta){
		int i=0;
		while(i<ARRAY_LEN && !(invitati[i]==risposta)){
			i++;
		}
		
		invitati[i]=0;
		
	}

    return 0;          
}