#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
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
int critic_time, closing_time;

//handler per SIGUSR1: per processi studente
void handler_sigusr1(int sig){
    #ifdef DEBUG
	//printf("Student (PID: %d). Bloccato. Aspetto il voto\n",getpid());
    #endif
    struct msgbuf message;
    int msg_id = msgget(IPC_KEY, 0666);
    msgrcv(msg_id,&message,sizeof(message.text),(long)(student->matricola+100),0);
    printf("Student (PID: %5d). Matricola: %3d. Voto_SO: %2d\n", getpid(),student->matricola, atoi(message.text));

    //FINE SIMULAZIONE
    shmdt(aula);

    exit(EXIT_SUCCESS);
}

//funzione richiamata per impostare l'handler di SIGUSR1
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
/*
#ifdef DEBUG
    printf("_Student (PID: %d). Parametri inizializzati\n",getpid());
    printf("_matricola: %s\n",argv[1]);
    printf("_prob_2: %s\n", argv[2]);
    printf("_prob_3: %s\n", argv[3]);
    printf("_nof_invites: %s\n", argv[4]);
    printf("_max_reject: %s\n", argv[5]);
#endif
*/
    //set handler
    sa_sigusr1();
    TEST_ERROR;
    sa_sigsegv(); //può capitare il segmentation fault nello student
    TEST_ERROR;

    //INIZIALIZZAZIONE IPC
    int sem_id, shm_id, msg_id;
    sem_id = semget(IPC_KEY, N_SEM, 0666);
    TEST_ERROR;
    msg_id = msgget(IPC_KEY, 0666);
    TEST_ERROR;
    shm_id = shmget(IPC_KEY,SHM_SIZE, 0666);
    TEST_ERROR;
    
    int matricola = atoi(argv[1]);
/*
#ifdef DEBUG
    printf("_Student (PID: %d). IPC agganciate\n",getpid());
#endif
* */
    //INIZIALIZZAZIONE MEMORIA CONDIVISA
    aula = (struct info_sim *)shmat(shm_id, NULL, 0666);
    TEST_ERROR;
    student = &(aula->student[matricola]);
    my_group = &(aula->group[matricola]);
    
    //ACQUISIZIONE PARAMETRI SIMULAZIONE
    int sim_time = aula->time_left;
    critic_time = sim_time*0.3; //30% residuo del tempo
    closing_time = sim_time*0.2; //20% del tempo residuo

    //INIZIALIZZAZIONE VARIABILI STUDENTE    
    float prob_2 = atoi(argv[2])/100.0,
          prob_3 = atoi(argv[3])/100.0;
    int nof_invites = atoi(argv[4]),
        max_reject = atoi(argv[5]);
        
    student->matricola = atoi(argv[1]);
    student->group = NOGROUP;
 /*   
#ifdef DEBUG
    printf("_Student (PID: %d). prima parte inizializzazione fatta\n",getpid());
#endif  
*/
    //inizializzazione voto_AdE
    srand(getpid());
    student->voto_AdE = rand()%13 + 18;  //compreso tra 18 e 30
    /*
#ifdef DEBUG
    printf("_Student (PID: %d). voto_AdE determinato\n",getpid());
#endif
*/  
    //inizializzazione nof_elems
    int val = rand()%POP_SIZE;
    if(val<POP_SIZE*prob_2)
        student->nof_elems = 2;
    else if(val>=POP_SIZE*prob_2 && val<POP_SIZE*(prob_2+prob_3))
        student->nof_elems = 3;
    else //if(val>=popsize*(prob_2+prob_3) && val<popsize)
        student->nof_elems = 4;
/*     
#ifdef DEBUG
    printf("_Student (PID: %d). nof_elems determinato\n",getpid());
#endif  
* */
    //INIZIALIZZAZIONE VARIABILI GRUPPO
    //inizialmente uno studente non fa parte di nessun gruppo
    my_group->n_members=0;
    my_group->is_closed=FALSE;
    my_group->max_voto=0;
    //non c'è bisogno di un semaforo perchè ogni studente scrive in un'area diversa
/*
#ifdef DEBUG
    printf("_Student (PID: %d). Student inizializzato\n"\
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
				"_my_group->max_voto: %d\n",
				
    getpid(),student->matricola, prob_2, prob_3, nof_invites, max_reject, student->group,
    student->voto_AdE, student->nof_elems, my_group->n_members, my_group->is_closed, my_group->max_voto);
#endif
  */ 
#ifdef DEBUG
    printf("_Student (PID: %d). Aspetto l'inizio della simulazione\n",getpid());
#endif 
    reserve_sem(sem_id, SEM_READY);
    reserve_sem(sem_id, SEM_GO);
    TEST_ERROR;
    
#ifdef DEBUG
    printf("_Student (PID: %d). Sbloccato\n",getpid());
#endif

    int is_leader=FALSE, accettato_invito=FALSE, n_invitati=0, n_rifiutati=0, i, hanno_risp=TRUE, close_group=FALSE;
    //array che contiene lo stato dell'invito mandato allo studente (LIBERO:non invitato, INVITATO, RISPOSTO: ha accettato o rifiutato)
    int invitati[POP_SIZE];
    //array che contiene lo stato degli inviti ricevuto da altri studenti (LIBERO:non sono stato invitato, INVITATO)
    int inviti[POP_SIZE];
    for(i=0;i<POP_SIZE;i++) {
	invitati[i]=LIBERO;
	inviti[i]=LIBERO;
    }
    
    //STRATEGIA INVITI
    while(1) {	
	hanno_risp = controllo_risposte(&is_leader, invitati, n_invitati, inviti); //se tutti hanno risposto ritorna true
	accettato_invito = rispondo_inviti(&accettato_invito, &is_leader, &n_rifiutati, max_reject, inviti, hanno_risp);
	close_group = chiudo_gruppo(&is_leader);

	if (aula->time_left >= closing_time && !accettato_invito && !close_group) {
	    algoritmo_inviti(invitati, &n_invitati, nof_invites); //determina chi invitare e aggiorna array invitati (DA_INVITARE)
	
	    //mando realmente gli inviti
	    int j;
	    for(j=0;j<POP_SIZE;j++) {
		if(invitati[j]==DA_INVITARE) {
		    //controllo se posso invitarlo
		    //if(reserve_sem_nowait(sem_id, j)==0) {
            reserve_sem(sem_id,j);
			/*#ifdef DEBUG
			printf("Valore semaforo %d: %d\n", j, get_sem_val2(sem_id, j));
			#endif*/
			invita_studente(j);
			invitati[j]=INVITATO;
			//release semaforo su chi ho invitato
			release_sem(sem_id, j);
		    //}
		    errno=0;
		    //altrimenti provo ad invitare gli altri
		    //altrimenti esco e vado a rispondere agli inviti
		}
	    }
	}
	
    }
}

void inizio_lettore(int sem_id){
    //sezione di entrata del modello lettori e scrittori
    reserve_sem(sem_id,MUTEX);
    aula->lettori++;
    if(aula->lettori==1)
        reserve_sem(sem_id,WRITE);
    release_sem(sem_id,MUTEX);
}

void fine_lettore(int sem_id){
    //sezione di uscita del modello lettori e scrittori
    reserve_sem(sem_id,MUTEX);
    aula->lettori--;
    if(aula->lettori==0)
        release_sem(sem_id,WRITE);
    release_sem(sem_id,MUTEX);
}

//controlla se ha ricevuto risposta agli inviti
//return true se tutti hanno risposto
//return false se qualcuno non ha risposto
int controllo_risposte(int *is_leader, int *invitati, int n_invitati, int *inviti) {
    struct msgbuf risposta;
    char messaggio[32];
    int mittente,
        msg_id = msgget(IPC_KEY,0666),
        sem_id = semget(IPC_KEY,N_SEM,0666);

    //reserve sul mio semaforo
    reserve_sem(sem_id, student->matricola);
    //controlla se ha ricevuto risposta agli inviti
    while(msgrcv(msg_id, &risposta, sizeof(risposta.text), (long)(student->matricola+100), IPC_NOWAIT)!=-1){ //i messaggi non vengono eliminati dalla coda: per permettere di leggere anche gli inviti
	//release semaforo del mio processo
    release_sem(sem_id,student->matricola);

    sscanf(risposta.text,"%s %d", messaggio, &mittente);

    inizio_lettore(sem_id);
	if(strcmp("Accetto",messaggio)==0 && my_group->is_closed==FALSE){
        fine_lettore(sem_id);

        reserve_sem(sem_id,WRITE);
	    inserisci_nel_mio_gruppo(mittente,is_leader);
        release_sem(sem_id,WRITE);

	    invitati[mittente]=RISPOSTO;
	    #ifdef DEBUG
		printf("Informazioni aggiornate gruppo n. %d\n"\
		       "n_members: %d\n"\
		       "is_closed: %d\n"\
		       "max_voto: %d\n\n",\
		       student->group,my_group->n_members, my_group->is_closed, my_group->max_voto);
	    #endif
	}
	else if(strcmp("Rifiuto",messaggio)==0 || (strcmp("Accetto",messaggio)==0 && my_group->is_closed==TRUE)){
	    fine_lettore(sem_id);
        invitati[mittente]=RISPOSTO;
    }
	else if(strcmp("Invito", messaggio)==0){
        fine_lettore(sem_id);
	    inviti[mittente]=INVITATO;
    }
	else {
        fine_lettore(sem_id);
	    fprintf(stderr, "%s: %d. Errore nella ricezione della risposta all'invito #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
	    exit(EXIT_FAILURE);
	}

    //reserve sul mio semaforo
    reserve_sem(sem_id, student->matricola);
    }
    //release semaforo del mio processo
    release_sem(sem_id,student->matricola);
    errno=0; //perchè la IPC_NOWAIT genera un errore se non trova nulla
    
    return hanno_risposto(invitati);
}

//controlla gli inviti ricevuti e li valuta
//return true se accetta un invito
//return false se non accetta
int rispondo_inviti(int *accettato, int *is_leader, int *n_rifiutati, int max_reject, int *inviti, int hanno_risp) {
    int matricola, sem_id=semget(IPC_KEY, N_SEM, 0666);

    for(matricola=0;matricola<POP_SIZE;matricola++) {
	    if(inviti[matricola]==INVITATO){
	       inviti[matricola]=LIBERO;

            inizio_lettore(sem_id);        
	        if(!hanno_risp)
		        rifiuta_invito(matricola); //ma non viene conteggiato in n_rifiutati
	        else if(*is_leader || *accettato || my_group->is_closed){
        		//il leader non può accettare inviti
        		//se ho già accettato un invito, gli altri li rifiuto
        		rifiuta_invito(matricola);
        		*n_rifiutati +=1;
	        }
    	    //valuto se accettare o meno
    	    /* accetta se non hai più rifiuti a disposizione */
    	    else if( (*n_rifiutati==max_reject) || \
    		/* oppure se il mittente ha lo stesso nof_elems */
    		(aula->student[matricola].nof_elems == student->nof_elems) || \
    		/* oppure se non ha lo stesso nof_elems, allora posso accettare inviti da studenti con il voto più alto del mio di 3 punti */
    		(aula->student[matricola].group==NOGROUP && aula->student[matricola].voto_AdE-3 >= student->voto_AdE) || \
    		(aula->group[matricola].n_members>1 && aula->group[matricola].max_voto-3 >= student->voto_AdE) || \
    		/* oppure se siamo nel critic_time, allora accetto qualunque invito */
    		(aula->time_left <= critic_time) ) {
    		    accetta_invito(matricola);
    		    *accettato=TRUE;
    	    }
    	    else {
    		    rifiuta_invito(matricola);
    		    *n_rifiutati +=1;
    	    }
            fine_lettore(sem_id);
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

void algoritmo_inviti(int *invitati, int *n_invitati, int nof_invites) {
    struct info_student *stud2;
    int i, max_invites, n=0;
    int sem_id=semget(IPC_KEY, N_SEM, 0666);
    
    //n conta gli inviti già inviati o da inviare (senza una risposta)
    for(i=0;i<POP_SIZE;i++) {
	if(invitati[i]==DA_INVITARE || invitati[i]==INVITATO) 
	    n++;
    }

    inizio_lettore(sem_id);
    if(student->group==NOGROUP)
	max_invites=(student->nof_elems-1); //tolgo 1 perchè sono io stesso
    else
        max_invites=student->nof_elems-my_group->n_members; //io sono incluso in n_members
    fine_lettore(sem_id);
    
    //non si possono invitare più studenti di quanti ne potrebbe contenere un gruppo
    for(i=0; i<POP_SIZE && n<max_invites && *n_invitati<nof_invites; i++) {
        stud2 = &(aula->student[i]);

        inizio_lettore(sem_id);
        //se sono dello stesso turno, non hanno un gruppo e non sono io stesso (imprescindibile per un invito)
        if(i!=student->matricola && stesso_turno(stud2, student) && stud2->group==NOGROUP && invitati[stud2->matricola]==LIBERO){
            //se hanno la stessa preferenza di nof_elems
            if(stud2->nof_elems==student->nof_elems) {
                //se stud2.voto > mio.voto
                if(stud2->voto_AdE > (student->voto_AdE)){
    		    invitati[stud2->matricola]=DA_INVITARE;
    		    n_invitati++;
    		    n++;
		        }
		        else if(aula->time_left <= critic_time) {
        	    invitati[stud2->matricola]=DA_INVITARE;
        	    n_invitati++;
        	    n++;
	            }
            }
    	    //se non hanno la mia stessa preferenza di nof_elems e siamo nel CRITIC TIME
    	    //invita qualunque studente pur di arrivare al nof_elems
    	    else if(aula->time_left <= critic_time){
    		invitati[stud2->matricola]=DA_INVITARE;
    		n_invitati++;
    		n++;
    	    }
        }
        fine_lettore(sem_id);
    }
}

//confronta le matricole di 2 studenti e verifica siano nello stesso turno
int stesso_turno (struct info_student *mat1, struct info_student *mat2) {
    return ((mat1->matricola)%2)==((mat2->matricola)%2);
}

//decido se conviene chiudere il gruppo(return true) oppure no(return false)
int chiudo_gruppo(int *is_leader) {
    int sem_id=semget(IPC_KEY, N_SEM, 0666);

    // (valutare se tenerlo)
    reserve_sem(sem_id, WRITE);
    //

    //se è già chiuso il gruppo, non si fa nulla
    if (my_group->is_closed);
    //se ho accettato un invito non posso fare nulla
    else if(!(*is_leader) && student->group!=NOGROUP);
    //quando devo chiudere il gruppo
    else if(my_group->n_members==4 || my_group->n_members==student->nof_elems || aula->time_left <= (closing_time)) {
	if(student->group==NOGROUP) {
	    //se devo chiudere il gruppo da solo
	    //creo gruppo
	    my_group->n_members=1;
	    my_group->max_voto=student->voto_AdE;
	    //modifico mie variabili studente
	    student->group=student->matricola;
	}
	//chiudo il gruppo (anche se manca poco tempo)
	my_group->is_closed=TRUE;
	*is_leader=TRUE;
	#ifdef DEBUG
	    printf("Il gruppo numero %d è stato chiuso dal leader.\n"\
		   "Informazioni aggiornate del gruppo\n"\
		   "n_members: %d\n"\
		   "is_closed: %d\n"\
		   "max_voto: %d\n\n",\
		   student->group,my_group->n_members, my_group->is_closed, my_group->max_voto);
	#endif
    }
    int closed = my_group->is_closed;

    // (valutare se tenerlo)
    release_sem(sem_id, WRITE);
    //

    return closed;
}

void inserisci_nel_mio_gruppo(int matricola, int *is_leader) {
    int sem_id = semget(IPC_KEY, N_SEM, 0666);

    if(student->group==NOGROUP){ //non esiste ancora il gruppo
	//creo gruppo
	my_group->n_members=2;
	my_group->max_voto=max(student->voto_AdE, aula->student[matricola].voto_AdE);
	//modifico mie variabili studente
	student->group=student->matricola;
	*is_leader=TRUE;
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

void invita_studente(int destinatario){
    struct msgbuf invito;
    int msg_id=msgget(IPC_KEY, 0666);
    
    invito.mtype = (long)(destinatario+100);//per evitare matricole=0
    sprintf(invito.text,"Invito %d", student->matricola);

    if(msgsnd(msg_id, &invito,sizeof(invito.text),0)==-1) {
        fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
	printf("Il Mittente : %d ha invitato il Destinatario %d\n",student->matricola,destinatario);
    #endif   
}

void rifiuta_invito(int mittente){
    //il destinatario dell'invito è student->matricola
    struct msgbuf rifiuto;
    int msg_id=msgget(IPC_KEY, 0666);
    
    rifiuto.mtype = (long)(mittente+100);
    sprintf(rifiuto.text,"Rifiuto %d", student->matricola);
    
    if(msgsnd(msg_id, &rifiuto, sizeof(rifiuto.text), 0)<0) {
	fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
	exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
	printf("Il Destinatario %d ha rifiutato l'invito del Mittente %d\n",student->matricola, mittente);
    #endif
    
}

void accetta_invito(int mittente){ 
    //il destinatario dell'invito è student->matricola
    struct msgbuf accetto;
    int msg_id = msgget(IPC_KEY, 0666);
    
    accetto.mtype = (long)(mittente+100); //mittente dell'invito
    sprintf(accetto.text,"Accetto %d", student->matricola);
    
    if(msgsnd(msg_id, &accetto, sizeof(accetto.text), 0)<0) {
	fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
	exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
	printf("Il Destinatario : %d ha accettato l'invito del Mittente %d\n",student->matricola, mittente);
    #endif
} 

//controlla che tutti gli invitati abbiano risposto agli inviti
int hanno_risposto(int *invitati){
    int ha_risposto = TRUE;
    int i;
    for(i=0;i<POP_SIZE && ha_risposto;i++){
        if(invitati[i]==INVITATO){
            ha_risposto = FALSE;
        }
    }
    return ha_risposto;
}
