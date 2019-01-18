#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../header/error.h"
#include "../header/conf_reader.h"

// FUNZIONE opt_control: restituisce 0 se i dati di opt.conf sono corretti. -1 altrimenti
static int opt_control(struct sim_opt* options){
    int result = 1, correct = 1;
 
    correct = options->prob_2>=0 && options->prob_2<=100;
    result &= correct;
    if(!correct)
        printf("ERRORE: %s, %d. Il valore di 'prob_2' non e' corretto o mancante (deve compreso tra 0 e 100)\n", __FILE__,__LINE__);
    
    correct = options->prob_3>=0 && options->prob_3<=100;
    result &= correct;
    if(!correct)
        printf("ERRORE: %s, %d. Il valore di 'prob_3' non e' corretto o mancante (deve compreso tra 0 e 100)\n", __FILE__,__LINE__);
    
    correct = options->prob_4>=0 && options->prob_4<=100;
    result &= correct;
    if(!correct)
        printf("ERRORE: %s, %d. Il valore di 'prob_4' non e' corretto o mancante (deve compreso tra 0 e 100)\n", __FILE__,__LINE__);
    
    correct = options->sim_time>=0 && options->sim_time<=TIME_LIMIT;
    result &= correct;
    if(!correct)
        printf("ERRORE: %s, %d. Il valore di 'sim_time' non e' corretto o mancante (deve compreso tra 0 e %d)\n",
            __FILE__,__LINE__,TIME_LIMIT);
    
    correct = options->nof_invites>=0 && options->nof_invites<=MAX_VALUE;
    result &= correct;
    if(!correct)
        printf("ERRORE: %s, %d. Il valore di 'nof_invites' non e' corretto o mancante (deve compreso tra 0 e %d)\n",
            __FILE__,__LINE__,MAX_VALUE);
    
    correct = options->max_reject>=0 && options->max_reject<=MAX_VALUE;
    result &= correct;
    if(!correct)
        printf("ERRORE: %s, %d. Il valore di 'max_reject' non e' corretto o mancante (deve compreso tra 0 e %d)\n",
            __FILE__,__LINE__,MAX_VALUE);
    
    correct = (options->prob_2 + options->prob_3 + options->prob_4) == 100;
    result &= correct;
    if(!correct)
        printf("ERRORE: %s, %d. I valori di 'prob_2' 'prob_3' 'prob_4' inseriti non hanno somma uguale a 100\n", __FILE__,__LINE__);

    return result;
}

// FUNZIONE init_options:   legge i dati in 'path' e li memorizza nella struttura 'options'
//
// VALORE DI RITORNO:
//   0 - se in 'path' sono presenti tutti i dati richiesti e con valori conformi
//  -1 - altrimenti
int init_options(struct sim_opt* options){
    FILE *fs = fopen("opt.conf", "r");
    TEST_ERROR;

    char key[20];   //memorizza la stringa di chiave
    int value;      //memorizza il valore

    //CICLO DI LETTURA
    //lettura chiavi e valori e controllo sulla conformita' dei dati presenti in path
    while(fscanf(fs,"%s %d",key,&value)!=EOF){
        if(!strcmp(key,"max_reject"))
            options->max_reject = value;
        else if(!strcmp(key,"nof_invites"))
            options->nof_invites = value;
        else if(!strcmp(key,"sim_time"))
            options->sim_time = value;
        else if(!strcmp(key,"prob_2"))
            options->prob_2 = value;
        else if(!strcmp(key,"prob_3"))
            options->prob_3 = value;
        else if(!strcmp(key,"prob_4"))
            options->prob_4 = value;
    }
    //EOF. Chiusura del file stream
    fclose(fs);
    TEST_ERROR;

    //CONTROLLO SULLA CORRETTEZZA DEI DATI
    if(!opt_control(options))
        return -1;
    else

    //giunti a questo punto del programma, tutti i controlli sono stati passati e 'options' contiene tutti i dati desiderati
    //la funzione in questo punto ritorna 0 che indica la corretta acquisizione dei dati di 'path'
        return 0;
}

/*

PER EVENTUALI TEST

int main(){
    struct sim_opt options;
    memset(&options,-1,sizeof(options));        //fondamentale che si inizializzi a -1
    if(init_options(&options)==-1)
        printf("Errore nell'inizializzazione!\n");
    else
        printf("sim_time: %d\nmax_reject: %d\nnof_invites: %d\nprob_2: %d\nprob_3: %d\nprob_4: %d\n",
                options.sim_time, options.max_reject, options.nof_invites, options.prob_2, options.prob_3, options.prob_4);
}

*/