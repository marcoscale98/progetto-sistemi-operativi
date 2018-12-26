#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include "header/stud.h"
#include "header/error.h"
#include "header/sem_util.h"
#include <sys/shm.h>

#define MSG_LEN 100

struct msgbuf{
    int mtype;             /* message type, sarà > 0 */
    char testo[MSG_LEN];    /* message testo */
};

int main(int argc,char *argv[]){ //argv[0]="student", argv[1]=matricola, argv[2]=prob_2, argv[3]=prob_3, argv[4]=nof_invites, argv[5]=max_reject, argv[6]=POPSIZE , shmid = argv[7])
    
    if(argc!=8){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }
    
    //INIZIALIZZAZIONE VARIABILI STUDENTE    
    struct info_student student;
    float prob_2 = atoi(argv[2])/100.0,
          prob_3 = atoi(argv[3])/100.0;
    int popsize = atoi(argv[6]),
        shmid = atoi(argv[7]),
        nof_invites = atoi(argv[4]),
        max_reject = atoi(argv[5]);
        
    student.matricola = atoi(argv[1]);
    student.group = NOGROUP; //non ancora in un gruppo
    //l'indice dei gruppi parte dall'1
    
    //inizializzazione voto_AdE
    srand(getpid());
    student.voto_AdE = rand()%13 + 18;  //compreso tra 18 e 30

    //inizializzazione nof_elems
    srand(getpid());
    int val = rand()%popsize;
    if(val<popsize*prob_2)
        student.nof_elems = 2;
    else if(val>=popsize*prob_2 && val<popsize*(prob_2+prob_3))
        student.nof_elems = 3;
    else //if(val>=popsize*(prob_2+prob_3) && val<popsize)
        student.nof_elems = 4;
     
    
    //INIZIALIZZAZIONE VARIABILI MIO GRUPPO (per ora ci sono solo io)
    struct info_group my_group;
    my_group.n_members=1;
    my_group.is_closed=0;
    my_group.pref_nof_elems=student.nof_elems;
    my_group.max_voto=student.voto_AdE;
    
    //INIZIALIZZAZIONE MEMORIA CONDIVISA
    struct classroom *aula = (struct classroom *)shmat(shmid, NULL, 0666);
    aula->students[matricola] = &student;
    aula->groups[matricola] = &my_group;
    //non c'è bisogno di un semaforo perchè non c'è sovrapposizione delle aree interessate
    
    //TESTING
    /*
    printf(" student.matricola: %d\n", student.matricola);
    printf(" prob_2: %f\n", prob_2);
    printf(" prob_3: %f\n", prob_3);
    printf(" nof_invites: %d\n", nof_invites);
    printf(" max_reject: %d\n", max_reject);
    printf(" popsize: %d\n", popsize);
    printf(" student.group: %d\n", student.group);
    printf(" student.voto_AdE: %d\n", student.voto_AdE);
    printf(" student.nof_elems: %d\n", student.nof_elems);
    printf(" shmid: %d\n", shmid);
    */
        
    //segnale di partenza
    
/* boolean esco = false;
 * 
 * while(segnale 20% residuo tempo || esco) {
 *     semaforo reserve
 *     int accetto = controllo_inviti();
 *     if(accetto)
 *         esco=true;
 *     else
 *         mando_inviti();
 *     semaforo release
 * } 
 * 
 * //cerco di chiudere tutti i gruppi
 * while(segnale di fine gara || esco) {
 *     semaforo reserve
 *     chiudo il gruppo
 *     semaforo release
 * }
 * 
 **/
    
    
    //STRATEGIA INIZIALE
    int i, counter_invites=0;
    struct info_student *stud2;
    
    for(i=0; i<popsize; i++) {
        stud2 = aula->students[i];
        
        //semaforo reserve
        
        //se sono dello stesso turno e non hanno un gruppo
        if(stesso_turno(&stud2, &student) && stud2->group==NOGROUP) {
            
            //se hanno la stessa preferenza di nof_elems
            if(stud2->nof_elems==student.nof_elems) {
                
                //se stud2.voto > mio.voto-3
                if(stud2->voto_AdE > (student.voto_AdE-3)) {
                    invita(stud2->matricola);
                    counter_invites;
                }
            }
        }
        //semaforo release
    }


//----------------------------------------------------------------------------------------



    if((msgget(ftok("opt.conf",0), SIRUSR | S_IWUSR)!-1); //utizzare ftok()
    	errExit("msgget")

    int counter_invites = 0;

    while(counter_invites < nof_invites & stud->group < stud->nof_elems){
    	if((msgrcv(id,&invito,MSGLEN,getpid(),IPC_NOWAIT)){
    		//se ci sono messaggi deve rispondere prima 
    	else if(info_student.group>-1){
    		rifiuta_invito();
    	else(){
    		invito_processo()
  			counter_invites ++;
  		}
  	}
  	
	
	void invito_processo(){

		char matricola[50] = argv[1];
		struct msgbuf invito;

		invito.mytype = //processo da invitare
		invito.testo = "Vuoi_Paratecipare_al_Gruppo? : ";

		strcat(invito.testo,matricola);

		if(msgsnd(id, &invito, MSG_LEN, 0)<0) {
    			fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
    			exit(EXIT_FAILURE);
    	}

    	TEST_ERROR
    	printf("Invito Spedito")

	}

	void rifiuta_invito(){

		struct msgbuf messaggio;
		int n_rifiuti=0;
		int pid_invitatore;

		while(msgrcv(id,&invito,MSGLEN,stud->matricola,IPC_NOWAIT)!=1){
			if(n_rifiuti<=max_reject){
				sscanf(invito.text,"%s : %d", messaggio_invito[50] , &pid_invitatore)
				messaggio.type= pid_invitatore;
				messaggio.testo="Rifiuto : "
				strcat(accetta_invito.testo, arg[1]); //matricola di chi accetta
				if(msgsnd(id, &accetta_invito, MSG_LEN, 0)<0) {
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
    return 0;          
}


//last.invite 
//poi leggo i messaggi se pid_acettatore o rifiutatore !=last.invite
