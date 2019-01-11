#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include "header/shm_util.h"
#include "header/error.h"
#include "header/config.h"
#include "header/sig_util.h"
#include "header/sem_util.h"
#include "header/stud.h"

struct info_student *student;
struct info_group *my_group;
struct info_sim *aula;
int msg_id;

//handler per SIGUSR1: per processi studente
void handler_sigusr1(int sig){
    printf("Student (PID: %d). Bloccato. Aspetto il voto\n",getpid());
    int msg_id;
    msg_id = msgget(IPC_KEY,0666);
    TEST_ERROR;

    struct msgbuf message;
    msgrcv(msg_id,&message,sizeof(message.text),student->matricola,0);
    printf("Student (PID: %d). voto_SO: %d\n", getpid(), atoi(message.text));

    //FINE SIMULAZIONE
    shmdt(aula);

    exit(EXIT_SUCCESS);
}

//funzione richiamata per impostare l'handler di SIGALRM
int sa_sigusr1(){
    struct sigaction sa;
    sa.sa_handler = handler_sigusr1;
    return sigaction(SIGUSR1,&sa,NULL);
}

//argv[0]="student", argv[1]=matricola, argv[2]=prob_2, argv[3]=prob_3, argv[4]=nof_invites, argv[5]=max_reject
int main(int argc,char *argv[]){

    if(argc!=6){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    printf("_Student (PID: %d). Parametri inizializzati\n",getpid());
    printf("_matricola: %s\n",argv[1]);
    printf("_prob_2: %s\n", argv[2]);
    printf("_prob_3: %s\n", argv[3]);
    printf("_nof_invites: %s\n", argv[4]);
    printf("_max_reject: %s\n", argv[5]);
#endif

    //set handler
    sa_sigusr1();
    TEST_ERROR;
    sa_sigsegv(); //può capitare il segmentation fault nello student
    TEST_ERROR;

    //INIZIALIZZAZIONE IPC
    int sem_id, shm_id;
    sem_id = semget(IPC_KEY, N_SEM, 0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY, 0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE, 0666);
    TEST_ERROR;
    
    int matricola = atoi(argv[1]);
    
#ifdef DEBUG
    printf("_Student (PID: %d). ipc agganciate\n",getpid());
#endif
    //INIZIALIZZAZIONE MEMORIA CONDIVISA
    aula = (struct info_sim *)shmat(shm_id, NULL, 0666);
    TEST_ERROR;
    student = &(aula->student[matricola]);
    my_group = &(aula->group[matricola]);
    //non c'è bisogno di un semaforo perchè non c'è sovrapposizione delle aree interessate
    
#ifdef DEBUG
    printf("_Student (PID: %d). Memoria condivisa inizializzata\n"\
	   "Indirizzo di memoria condivisa di student: %p\n"\
	   "Indirizzo di memoria condivisa di my_group: %p\n",\
	   getpid(), student, my_group);
#endif

    //INIZIALIZZAZIONE VARIABILI STUDENTE    
    float prob_2 = atoi(argv[2])/100.0,
          prob_3 = atoi(argv[3])/100.0;
    int nof_invites = atoi(argv[4]),
        max_reject = atoi(argv[5]);
        
    student->matricola = atoi(argv[1]);
    student->group = NOGROUP;
    student->leader=FALSE;
    
#ifdef DEBUG
    printf("_Student (PID: %d). prima parte inizializzazione fatta\n",getpid());
#endif  

    //inizializzazione voto_AdE
    srand(getpid());
    student->voto_AdE = rand()%13 + 18;  //compreso tra 18 e 30
#ifdef DEBUG
    printf("_Student (PID: %d). voto_AdE determinato\n",getpid());
#endif  
    //inizializzazione nof_elems
    int val = rand()%POP_SIZE;
    if(val<POP_SIZE*prob_2)
        student->nof_elems = 2;
    else if(val>=POP_SIZE*prob_2 && val<POP_SIZE*(prob_2+prob_3))
        student->nof_elems = 3;
    else //if(val>=popsize*(prob_2+prob_3) && val<popsize)
        student->nof_elems = 4;
     
#ifdef DEBUG
    printf("_Student (PID: %d). nof_elems determinato\n",getpid());
#endif  
    //INIZIALIZZAZIONE VARIABILI GRUPPO
    //inizialmente uno studente non fa parte di nessun gruppo
    my_group->n_members=0;
    my_group->is_closed=FALSE;
    my_group->pref_nof_elems=0;
    my_group->max_voto=0;

#ifdef DEBUG
    printf("_Student (PID: %d).Student inizializzato\n"\
				"_student->matricola: %d\n"\
				"_prob_2: %f\n"\
				"_prob_3: %f\n"\
				"_nof_invites: %d\n"\
				"_max_reject: %d\n"\
				"_student->group: %d\n"\
				"_student->voto_AdE: %d\n"\
				"_student->nof_elems: %d\n"\
				"_my_group->n_members: %d\n"\
				"_my_group->is_closed: %d \n"\
				"_my_group->pref_nof_elems: %d\n"\
				"_my_group->max_voto: %d\n",
				
    getpid(),student->matricola, prob_2, prob_3, nof_invites, max_reject, student->group,
    student->voto_AdE, student->nof_elems, my_group->n_members, my_group->is_closed, my_group->pref_nof_elems, my_group->max_voto);
#endif
   
#ifdef DEBUG
    printf("_Student (PID: %d). Effettuo reserve_sem\n",getpid());
#endif 
    reserve_sem(sem_id, SEM_READY);
    TEST_ERROR;
    
#ifdef DEBUG
    printf("_Student (PID: %d). Sbloccato\n",getpid());
#endif

    int accettato_invito = FALSE;
    int invitati[nof_invites];
    int n_invitati=0;
    int n_rifiutati=0;
    memset(invitati,-1,sizeof(invitati));
    
/***********************************************************************
 * 
 * GLI STUDENTI RIMANGONO BLOCCATI NEL WHILE
 * 
 * ********************************************************************/
#ifdef DEBUG
    printf("Valore di aula->time_left: %d\n", aula->time_left);
#endif

    //STRATEGIA INVITI
    while(aula->time_left > 0) {
        reserve_sem(sem_id, SEM_SHM);
	//#ifdef DEBUG
	    //printf("Student (PID: %d) può mandare gli inviti\n", getpid());
	//#endif
	if (controllo_risposte(invitati, n_invitati)) {//se tutti hanno risposto
	    //se leader true rifiuta gli inviti
	    //se ho già accettato un invito, rifiuto i successivi
	    //se il mio gruppo è chiuso, rifiuto gli inviti
	    accettato_invito = rispondo_inviti(&accettato_invito, &n_rifiutati, max_reject);

	    if (!accettato_invito && !chiudo_gruppo()) {
		mando_inviti(invitati, &n_invitati, nof_invites);
	    }
	}
	//#ifdef DEBUG
	    //printf("Student (PID: %d) ha finito il suo giro di inviti\n", getpid());
	//#endif
	release_sem(sem_id, SEM_SHM);
    } 
 
    //FINE SIMULAZIONE
    sleep(5); //aspetta il voto dal gestore
    exit(EXIT_FAILURE);
}

//controlla se ha ricevuto risposta agli inviti
//return true se tutti hanno risposto
//return false se qualcuno non ha risposto
int controllo_risposte(int *invitati, int n_invitati) {
    struct msgbuf risposta;
    char messaggio[50];
    int mittente;
    
    //controlla se ha ricevuto risposta agli inviti
    while(msgrcv(msg_id, &risposta, MSG_LEN, student->matricola, IPC_NOWAIT)!=-1){
	sscanf(risposta.text,"%s : %d", messaggio, &mittente);
	if(strcmp("Accetto",messaggio)==0){
	    inserisci_nel_mio_gruppo(mittente);
	    setta_risposta(mittente, invitati, n_invitati);
	    #ifdef DEBUG
		printf("Informazioni aggiornate gruppo n. %d\n"\
		       "n_members: %d\n"\
		       "is_closed: %d\n"\
		       "max_voto: %d\n\n",\
		       student->group,my_group->n_members, my_group->is_closed, my_group->max_voto);
	    #endif
	}
	else if(strcmp("Rifiuto",messaggio)==0){
	    setta_risposta(mittente, invitati, n_invitati);
	}
    }
    return hanno_risposto(invitati, n_invitati);

}

//controlla gli inviti ricevuti e li valuta
//return true se accetta un invito
//return false se non accetta
int rispondo_inviti(int *accettato, int *n_rifiutati, int max_reject) {
    struct msgbuf invito;
    char messaggio[50];
    int mittente;
    
    //controlla se ha ricevuto degli inviti
    while(msgrcv(msg_id, &invito,MSG_LEN,student->matricola,IPC_NOWAIT)!=-1){
	sscanf(invito.text,"%s : %d", messaggio, &mittente);
	if(student->leader || *accettato || my_group->is_closed) {
	    //il leader non può accettare inviti
	    //se ho già accettato un invito, gli altri li rifiuto
	    rifiuta_invito(mittente, n_rifiutati);
	}
	else {  //valuto se accettare o meno
	    /* accetta se non hai più rifiuti a disposizione */
	    if( (*n_rifiutati==max_reject) || \
	    
	    /* oppure se il mittente ha lo stesso nof_elems */
	    (aula->student[mittente].nof_elems == student->nof_elems) || \
	    
	    /* oppure se non ha lo stesso nof_elems, allora posso accettare inviti da studenti con il voto più alto del mio di 3 punti */
	    (aula->student[mittente].group==NOGROUP && aula->student[mittente].voto_AdE-3 >= student->voto_AdE) || \
	    (aula->group[mittente].n_members>1 && aula->group[mittente].max_voto-3 >= student->voto_AdE) || \
	    
	    /* oppure se siamo nel CRITIC_TIME, allora accetto qualunque invito */
	    (aula->time_left <= CRITIC_TIME) ) {
		accetta_invito(mittente);
		*accettato=TRUE;
	    }
	    else
		rifiuta_invito(mittente, n_rifiutati);
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

void mando_inviti(int *invitati, int *n_invitati, int nof_invites) {
    struct info_student *stud2;
    int i;
    for(i=0; i<POP_SIZE; i++) {
        stud2 = &(aula->student[i]);
        
        //se sono dello stesso turno, non hanno un gruppo e ho ancora inviti a disposizione (imprescindibile per un invito)
        if(stesso_turno(stud2, student) && stud2->group==NOGROUP && *n_invitati<nof_invites) {
            
            //se hanno la stessa preferenza di nof_elems
            if(stud2->nof_elems==student->nof_elems) {
                
                //se stud2.voto > mio.voto
                if(stud2->voto_AdE > (student->voto_AdE))
                    invita_studente(stud2->matricola, invitati, n_invitati);
		else if(aula->time_left <= CRITIC_TIME)
		    invita_studente(stud2->matricola, invitati, n_invitati);
            }
	    
	    //se non hanno la mia stessa preferenza di nof_elems e siamo nel CRITIC TIME
	    else if(aula->time_left <= CRITIC_TIME) {
		//invita qualunque studente pur di arrivare al nof_elems
		invita_studente(stud2->matricola, invitati, n_invitati);
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
    
    if(my_group->n_members==4 || my_group->n_members==student->nof_elems || aula->time_left <= (CRITIC_TIME)) {
	if(my_group->n_members<=1) {
	    //se devo chiudere il gruppo da solo
	    //creo gruppo
	    my_group->n_members=1;
	    my_group->max_voto=student->voto_AdE;
	    //modifico mie variabili studente
	    student->group=student->matricola;
	    student->leader=TRUE;
	}
	//chiudo il gruppo (anche se manca poco tempo)
	my_group->is_closed=TRUE;
	student->leader=TRUE;
    }
    //se è già chiuso il gruppo, non si fa nulla
    return my_group->is_closed;
}

void inserisci_nel_mio_gruppo(int matricola) {
    
    if(my_group->n_members<=1){ //non esiste ancora il gruppo
	//creo gruppo
	my_group->n_members=2;
	my_group->max_voto=max(student->voto_AdE, aula->student[matricola].voto_AdE);
	//modifico mie variabili studente
	student->group=student->matricola;
	student->leader=TRUE;
	//modifico variabili altro student
	aula->student[matricola].group=student->matricola;
    }
    else if(my_group->n_members<4) {
	//modifichiamo i campi dello studente "matricola"
	aula->student[matricola].group = student->group;
	//modifico i campi del gruppo
	my_group->n_members += 1;
	my_group->max_voto = max(my_group->max_voto, aula->student[matricola].voto_AdE);
    } else {
	fprintf(stderr, "%s: %d. Il gruppo non può essere formato da più di 4 studenti #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
	exit(EXIT_FAILURE);
    }
}

void invita_studente(int destinatario, int *invitati, int *n_invitati){
    struct msgbuf invito;
    invito.mtype = destinatario;
    sprintf(invito.text,"Invito : ");

    char buf[8];
    sprintf(buf,"%d",student->matricola);
    strcat(invito.text,buf);

    if(msgsnd(msg_id, &invito,MSG_LEN,0)==-1) {
        fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
	printf("Il Mittente : %d ha invitato il Destinatario %d",student->matricola,destinatario);
    #endif
    invitati[*n_invitati] = destinatario;
    *n_invitati +=1;
}

void rifiuta_invito(int mittente, int *n_rifiutati){
    //il destinatario dell'invito è student->matricola
    struct msgbuf rifiuto;
//    char messaggio[50];

//    char buf[8];
//    sprintf(buf,"%d",mittente);

//    while(msgrcv(msg_id,&invito,MSG_LEN,mittente,IPC_NOWAIT)!=1){
        /*if(n_rifiuti<=max_reject){ Mettere nel Main */
//            sscanf(invito.text,"%s : %d", messaggio, &destinatario);
            rifiuto.mtype = mittente;
            sprintf(rifiuto.text,"Rifiuto : %d", student->matricola);
 //           strcat(invito.text, buf); //matricola di chi accetta
            if(msgsnd(msg_id, &rifiuto, MSG_LEN, 0)<0) {
                fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            TEST_ERROR
	    #ifdef DEBUG
		printf("Il Destinatario %d ha rifiutato l'invito del Mittente %d",student->matricola, mittente);
	    #endif
            *n_rifiutati +=1;

   // }
}

void accetta_invito(int mittente){ //il destinatario dell'invito è student->matricola
    
//    char messaggio[50];
    struct msgbuf accetto;

    char buf[8];
    sprintf(buf,"%d",mittente);

//    while(msgrcv(msg_id,&invito,MSG_LEN,mittente,IPC_NOWAIT)!=-1){
//        sscanf(invito.text,"%s : %d", messaggio,&destinatario);
        accetto.mtype = mittente;
        sprintf(accetto.text,"Accetto : %d", student->matricola);
//        strcat(invito.text,buf); //buf = matricola di chi accetta
        if(msgsnd(msg_id, &accetto, MSG_LEN, 0)<0) {
            fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
            exit(EXIT_FAILURE);
	}
	#ifdef DEBUG
            printf("Il Destinatario : %d ha accettato l'invito del Mittente %d",student->matricola, mittente);
	#endif
//	}
} 

//controlla che tutti gli invitati abbiano risposto agli inviti
int hanno_risposto(int *invitati, int n_invitati){
    int ha_risposto = TRUE;
    int i;
    for(i=0;i<n_invitati && ha_risposto;i++){
        if(invitati[i]==-1){
            ha_risposto = FALSE;
        }
    }
    return ha_risposto;
}

//funzione che imposta a -1 l'elemento che contiene la matricola uguale a mittente
void setta_risposta(int mittente, int *invitati, int n_invitati){
    int i=0;
    while(i<n_invitati && invitati[i]!=mittente){
        i++;
    }
    invitati[i]=-1;
}




