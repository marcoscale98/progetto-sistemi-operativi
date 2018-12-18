#ifndef _CONF_READER_H_
#define _CONF_READER_H_

#define POPSIZE 10
#define MAX_VALUE 20
#define TIME_LIMIT 120

//struttura che contiene tutti i dati da leggere in opt.conf
struct sim_opt{
    int prob_2;
    int prob_3;
    int prob_4;
    int nof_invites;
    int max_reject;
    int sim_time;
};

// FUNZIONE opt_control: restituisce 0 se i dati di opt.conf sono corretti. -1 altrimenti
static int opt_control(struct sim_opt* options);

// FUNZIONE init_options:   legge i dati in 'path' e li memorizza nella struttura 'options'. Restituisce:
//      0 - se in 'path' sono presenti tutti i dati richiesti e con valori conformi
//      -1 - altrimenti
int init_options(struct sim_opt* options);

#endif