#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "header/shm_util.h"
#include "header/error.h"
#include "header/config.h"
#include "header/sig_util.h"
#include "header/sem_util.h"
#include "header/msg_util.h"
#include "module/msg_util.c"

#define ARRAY_LEN
#define DEBUG

extern struct info_student *student;
extern struct info_group *my_group;

int main(int argc,char *argv[]){ //argv[0]="student", argv[1]=matricola, argv[2]=prob_2, argv[3]=prob_3, argv[4]=nof_invites, argv[5]=max_reject

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
    
    //INIZIALIZZAZIONE MEMORIA CONDIVISA
    struct info_sim *aula = (struct info_sim *)shmat(shm_id, NULL, 0666);
    student = aula->student[student.matricola];
    //non c'è bisogno di un semaforo perchè non c'è sovrapposizione delle aree interessate
    
    //INIZIALIZZAZIONE VARIABILI STUDENTE    
    //struct info_student student;
    float prob_2 = atoi(argv[2])/100.0,
          prob_3 = atoi(argv[3])/100.0;
    int nof_invites = atoi(argv[4]),
        max_reject = atoi(argv[5]);
        
    student->matricola = atoi(argv[1]);
    student->group = NOGROUP; //non ancora in un gruppo
    //l'indice dei gruppi parte dall'1
    student->leader=FALSE;
    
    //inizializzazione voto_AdE
    srand(getpid());
    student->voto_AdE = rand()%13 + 18;  //compreso tra 18 e 30

    //inizializzazione nof_elems
    srand(getpid());
    int val = rand()%POP_SIZE;
    if(val<POP_SIZE*prob_2)
        student->nof_elems = 2;
    else if(val>=POP_SIZE*prob_2 && val<POP_SIZE*(prob_2+prob_3))
        student->nof_elems = 3;
    else //if(val>=popsize*(prob_2+prob_3) && val<popsize)
        student->nof_elems = 4;
     
    
    //INIZIALIZZAZIONE VARIABILI MIO GRUPPO (per ora ci sono solo io)
    //struct info_group my_group;
    my_group.n_members=1;
    my_group.is_closed=FALSE;
    my_group.pref_nof_elems=student->nof_elems;
    my_group.max_voto=student->voto_AdE;
    
    //INIZIALIZZAZIONE MEMORIA CONDIVISA
    struct info_sim *aula = (struct info_sim *)shmat(shm_id, NULL, 0666);
    aula->student[student->matricola] = student;
    aula->group[student->matricola] = my_group;
    //non c'è bisogno di un semaforo perchè non c'è sovrapposizione delle aree interessate
    
#ifdef DEBUG
    printf("PID: %d\nstudent->matricola: %d\n", getpid(),student->matricola);
    printf("prob_2: %f\n", prob_2);
    printf("prob_3: %f\n", prob_3);
    printf("nof_invites: %d\n", nof_invites);
    printf("max_reject: %d\n", max_reject);
    printf("student->group: %d\n", student->group);
    printf("student->voto_AdE: %d\n", student->voto_AdE);
    printf("student->nof_elems: %d\n", student->nof_elems);
#endif
        
    reserve_sem(sem_id, SEM_READY);
    TEST_ERROR;
    
    int accettato_invito = FALSE;
    int invitati[ARRAY_LEN];
    for(int j=0; j<ARRAY_LEN; j++)
        invitati[j]=-1;
    
    while(aula->time_left <= (0.5*POP_SIZE) || accettato_invito) {
        reserve_sem(sem_id, SEM_SHM);
 
	if (controllo_risposte(invitati, aula);) {//se tutti hanno risposto
	    accettato_invito = controllo_inviti(leader); //se leader true rifiuta gli inviti
	    //se accetto true, esco
	    if (!accettato_invito)
		mando_inviti();
	}
	release_sem(sem_id, SEM_SHM);
    } 
 
    //cerco di chiudere tutti i gruppi
    while(aula->time_left==0 || accettato_invito) {
        reserve_sem(sem_id, SEM_SHM);
        chiudo_gruppo()
        release_sem(sem_id, SEM_SHM);
    }
    
/*    
    //STRATEGIA INIZIALE
    int i, counter_invites=0;
    struct info_student *stud2;
    
    for(i=0; i<POP_SIZE; i++) {
        stud2 = aula->students[i];
        
        //semaforo reserve
        
        //se sono dello stesso turno e non hanno un gruppo
        if(stesso_turno(&stud2, &student) && stud2->group==NOGROUP) {
            
            //se hanno la stessa preferenza di nof_elems
            if(stud2->nof_elems==student->nof_elems) {
                
                //se stud2.voto > mio.voto-3
                if(stud2->voto_AdE > (student->voto_AdE-3)) {
                    invita(stud2->matricola);
                    counter_invites;
                }
            }
        }
        //semaforo release
    }
*/
}

//controlla se ha ricevuto risposta agli inviti
//return true se tutti hanno risposto
//return false se qualcuno non ha risposto
int controllo_risposte(int *invitati, struct info_sim *aula) {
    struct msgbuf risposta;
    char messaggio[50];
    int mittente;
    struct info_student student;

    //controlla se ha ricevuto risposta agli inviti
    while(msgrcv(msg_id,&risposta,MSGLEN,student->matricola,IPC_NOWAIT)!=-1){
	sscanf(risposta.text,"%s : %d", messaggio[50], &mittente);
	
	if(strcmp("Accetto",messaggio)==0){
	    if (student->group==NOGROUP) { //se non si è ancora formato il gruppo
		//lo scrive in memoria condivisa
		int j=0;
		for(; j<POP_SIZE && aula->group[j]!=NOGROUP; j++);
		my_group = aula->group[j];
		my_group->n_members=2;
		my_group->is_closed=FALSE;
		my_group->max_voto=max(student->voto_AdE, aula->student[mittente]);
		student->leader=TRUE;
	    }
	    else { //se c'era già il gruppo
		my_group->n_members+=1;
		my_group->max_voto=max(my_group->max_voto, aula->student[mittente]);
	    }
	    setta_risposta(invitati,mittente);
	}
	else if(strcmp("Rifiuto",messaggio)==0){
	    setta_risposta(invitati,mittente);
	}
    }
    return hanno_risposto(invitati);
    /*
	else{ //ricezione altro invito 
	    if(hanno_risposto(invitati)) //se tutti hanno risposto
		    //Scegliere Processo da Invitare per passarlo alla funzione di invito
		    invita_processo(invitati);
	    else
		    rifiuta_invito();
	}
    }
    * */

}

void controllo_inviti(leader) {
    
    
}

void invita_processo(int *invitati, int mittente, int destinatario, int msg_id){
    struct msgbuf invito;
    static int i = 0;
    
    invito.mtype = (long)destinatario;
    sprintf(invito.text,"Invito :");

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

void rifiuta_invito(int mittente, int msg_id, int n_rifiuti,int max_reject){

    struct msgbuf invito;
    int destinatario;
    char messaggio[50];

    char buf[8];
    sprintf(buf,"%d",mittente);

    while(msgrcv(msg_id,&invito,MSG_LEN,mittente,IPC_NOWAIT)!=1){
        if(n_rifiuti<=max_reject){
            sscanf(invito.text,"%s : %d", messaggio, &destinatario);
            invito.mtype=(long)destinatario;
            sprintf(invito.text,"Rifiuto :");
            strcat(invito.text, buf); //matricola di chi accetta
            if(msgsnd(msg_id, &messaggio, MSG_LEN, 0)<0) {
                fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            TEST_ERROR
            printf("Invito Spedito");
            n_rifiuti ++;
        }else{
           //accetta_invito();
        }
    }
}

void accetta_invito(int mittente , int destinatario , int msg_id){

    char messaggio[50];
    struct msgbuf invito;

    char buf[8];
    sprintf(buf,"%d",mittente);
    strcat(invito.text,buf);

    while(msgrcv(msg_id,&invito,MSG_LEN,mittente,IPC_NOWAIT)!=-1){
        sscanf(invito.text,"%s : %d", messaggio,&destinatario);
        invito.mtype=(long)destinatario;
        sprintf(invito.text,"Accetto :");
        strcat(invito.text,buf); //buf = matricola di chi accetta
        if(msgsnd(msg_id, &accetta_invito, MSG_LEN, 0)<0) {
            fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        else
            printf("Invito Spedito");
    }
} 

//controlla che tutti gli invitati abbiano risposto agli inviti
int hanno_risposto(int *invitati){
    int return_value = TRUE;
    int i;
    for(i=0;i<ARRAY_LEN;i++){
        if(invitati[i]!=-1){
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




