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
#include "header/stud.h"
#include "module/sem_util.c"

#define ARRAY_LEN 10
#define DEBUG

struct info_student *student;
struct info_group *my_group;
struct info_sim *aula;

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
    
    int matricola = atoi(argv[1]);
    
    //INIZIALIZZAZIONE MEMORIA CONDIVISA
    aula = (struct info_sim *)shmat(shm_id, NULL, 0666);
    student = &(aula->student[matricola]);
    my_group = aula->group[matricola];   
    //non c'è bisogno di un semaforo perchè non c'è sovrapposizione delle aree interessate
    
    //INIZIALIZZAZIONE VARIABILI STUDENTE    
    float prob_2 = atoi(argv[2])/100.0,
          prob_3 = atoi(argv[3])/100.0;
    int nof_invites = atoi(argv[4]),
        max_reject = atoi(argv[5]);
        
    student->matricola = atoi(argv[1]);
    //ogni studente inizialmente fa parte di un gruppo in cui e' da solo
    student->group = matricola;
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
    my_group->n_members=1;
    my_group->is_closed=FALSE;
    my_group->pref_nof_elems=student->nof_elems;
    my_group->max_voto=student->voto_AdE;
    
/*  //INIZIALIZZAZIONE MEMORIA CONDIVISA
    struct info_sim *aula = (struct info_sim *)shmat(shm_id, NULL, 0666);
    aula->student[student->matricola] = student;
    aula->group[student->matricola] = my_group;
    //non c'è bisogno di un semaforo perchè non c'è sovrapposizione delle aree interessate
*/ 
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
    int n_invitati=0;
    int n_rifiutati=0;
    for(int j=0; j<ARRAY_LEN; j++)
        invitati[j]=-1;
    
    
    //STRATEGIA INVITI
    while(aula->time_left > 0) {
        reserve_sem(sem_id, SEM_SHM);
	
	if (controllo_risposte(msg_id, invitati)) {//se tutti hanno risposto
	    //se leader true rifiuta gli inviti
	    //se ho già accettato un invito, rifiuto i successivi
	    //se il mio gruppo è chiuso, rifiuto gli inviti
	    accettato_invito = rispondo_inviti(msg_id, &accettato_invito);

	    if (!accettato_invito && !chiudo_gruppo()) {
		mando_inviti(msg_id, invitati);
	    }
	}
	
	release_sem(sem_id, SEM_SHM);
    } 
 
    
}

//controlla se ha ricevuto risposta agli inviti
//return true se tutti hanno risposto
//return false se qualcuno non ha risposto
int controllo_risposte(int msg_id, int *invitati) {
    struct msgbuf risposta;
    char messaggio[50];
    int mittente;
    extern struct info_student *student;
    extern struct info_group *my_group;
    
    //controlla se ha ricevuto risposta agli inviti
    while(msgrcv(msg_id, &risposta, MSG_LEN, student->matricola, IPC_NOWAIT)!=-1){
	sscanf(risposta.text,"%s : %d", messaggio, &mittente);
	if(strcmp("Accetto",messaggio)==0){
	    inserisci_nel_mio_gruppo(mittente);
	    setta_risposta(invitati,mittente);
	    #ifdef DEBUG
		printf("Informazioni aggiornate gruppo %d\n", student->group);
		printf("n_members: %d\n", my_group->n_members);
		printf("is_closed: %d\n", my_group->is_closed);
		printf("max_voto: %d\n\n", my_group->max_voto);
	    #endif
	}
	else if(strcmp("Rifiuto",messaggio)==0){
	    setta_risposta(invitati,mittente);
	}
    }
    return hanno_risposto(invitati);

}

//controlla gli inviti ricevuti e li valuta
//return true se accetta un invito
//return false se non accetta
int rispondo_inviti(int msg_id, int *accettato) {
    struct msgbuf invito;
    char messaggio[50];
    int mittente;
    extern struct info_student *student;
    extern struct info_group *my_group;
    extern struct info_sim *aula;
    
    //controlla se ha ricevuto degli inviti
    while(msgrcv(msg_id,&invito,MSG_LEN,student->matricola,IPC_NOWAIT)!=-1){
	sscanf(invito.text,"%s : %d", messaggio, &mittente);
	if(student->leader || *accettato || my_group->is_closed) 
	    //il leader non può accettare inviti
	    //se ho già accettato un invito, gli altri li rifiuto
	    rifiuta_invito(msg_id, mittente, n_rifiutati, max_reject);
	    
	else {  //valuto se accettare o meno
	    if(n_rifiutati==max_reject || (aula->group[mittente]->max_voto >= student->voto_AdE && aula->group[mittente]->n_members <= student->nof_elems-1)) {
	        accetta_invito(student->matricola, mittente, msg_id);
		*accettato=TRUE;
	    }
	    else
		rifiuta_invito(msg_id, mittente, n_rifiuti, max_reject);
	}
    }
    return *accettato;
}

int max(int num1, int num2) {
    if(num1>num2)
        return num1;
    else
	return num2;
    
}

void mando_inviti(int msg_id, int *invitati) {
    struct info_student *stud2;
    int i;
    for(i=0; i<POP_SIZE; i++) {
        stud2 = &(aula->student[i]);
        
        //se sono dello stesso turno e non hanno un gruppo (imprescindibile per un invito)
        if(stesso_turno(stud2, student) && stud2->group==NOGROUP && n_invitati<nof_invites) {
            
            //se hanno la stessa preferenza di nof_elems
            if(stud2->nof_elems==student->nof_elems) {
                
                //se stud2.voto > mio.voto
                if(stud2->voto_AdE > (student->voto_AdE)) {
                    invita_studente(invitati, student->matricola, stud2->matricola, msg_id);
                }

            }
	    //se non hanno la mia stessa preferenza di nof_elems
	    else {
		//se stud2.voto > mio.voto-3
                if(stud2->voto_AdE > (student->voto_AdE-3)) {
                    invita_studente(invitati, student->matricola, stud2->matricola, msg_id);
                }
	    }
        }
    }    
    
    
}

//confronta le matricole di 2 studenti e verifica siano nello stesso turno
int stesso_turno (struct info_student *mat1, struct info_student *mat2) {
    return ((mat1->matricola)%2)==((mat2->matricola)%2);
}

//decido se conviene chiudere il gruppo(return true) oppure no(return false)
int chiudo_gruppo() {
    extern struct info_student *student;
    extern struct info_group *my_group;
    extern struct info_sim *aula;
    
    if(my_group->n_members==4 || my_group->n_members==student->nof_elems || aula->time_left <= (0.5*POP_SIZE)) {
	//chiudo il gruppo (anche se manca poco tempo)
	my_group->is_closed=TRUE;
	student->leader=TRUE;
    }
    return my_group->is_closed;
}

void inserisci_nel_mio_gruppo(int matricola) {
    extern struct info_student *student;
    extern struct info_group *my_group;
    
    //modifichiamo i campi dello studente "matricola"
    aula->student[matricola].group = student->group;
    aula->group[matricola]=NULL; //non esiste più il precedente gruppo
    //modifico i campi del gruppo
    my_group->n_members += 1;
    my_group->max_voto = max(student->voto_AdE, aula->student[matricola].voto_AdE);
    //modifico il campo di student
    student->leader=TRUE;
}

void invita_studente(int *invitati, int mittente, int destinatario, int msg_id){
    struct msgbuf invito;
    static int i = 0;
    
    invito.mtype = (long)destinatario;
    sprintf(invito.text,"Invito : ");

    char buf[8];
    sprintf(buf,"%d",mittente);
    strcat(invito.text,buf);

    if(msgsnd(msg_id,&invito,MSG_LEN,0)==-1) {
        fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Il Mittente : %d ha invitato il Destinatario %d",mittente,destinatario);
    invitati[i]=destinatario;
    i++;
}



void rifiuta_invito(int mittente, int msg_id , int destinatario /*int n_rifiuti,int max_reject**/){

    struct msgbuf invito;
    char messaggio[50];

    char buf[8];
    sprintf(buf,"%d",mittente);

    while(msgrcv(msg_id,&invito,MSG_LEN,mittente,IPC_NOWAIT)!=1){
        /*if(n_rifiuti<=max_reject){ Mettere nel Main */
            sscanf(invito.text,"%s : %d", messaggio, &destinatario);
            invito.mtype=(long)destinatario;
            sprintf(invito.text,"Rifiuto :");
            strcat(invito.text, buf); //matricola di chi accetta
            if(msgsnd(msg_id, &messaggio, MSG_LEN, 0)<0) {
                fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            TEST_ERROR
            printf("Il Destinatario : %d ha rifiutato l'invito del Mittente %d",mittente,destinatario);
            n_rifiuti ++;
        /*}else{
           //accetta_invito();
        } Mettere nel Main */
    }
}

void accetta_invito(int mittente , int destinatario , int msg_id){

    char messaggio[50];
    struct msgbuf invito;

    char buf[8];
    sprintf(buf,"%d",mittente);

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
            printf("Il Destinatario : %d ha accettato l'invito del Mittente %d",mittente,destinatario);
    }
} 

//controlla che tutti gli invitati abbiano risposto agli inviti
int hanno_risposto(int *invitati){
    int ha_risposto = TRUE;
    int i;
    for(i=0;i<ARRAY_LEN && ha_risposto;i++){
        if(invitati[i]==-1){
            ha_risposto = FALSE;
        }
    }
    return ha_risposto;
}

//funzione che imposta a -1 l'elemento che contiene la matricola uguale a mittente
void setta_risposta(int *invitati,int mittente){
    int i=0;
    while(i<ARRAY_LEN && !invitati[i]==mittente){
        i++;
    }
    invitati[i]=-1;
}




