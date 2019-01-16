#ifndef _SHM_UTIL_H_
#define _SHM_UTIL_H_

#define NOGROUP -1
#define SHM_SIZE sizeof(struct info_sim)

#include "config.h"

struct info_student{
    //matricola dello studente associato alla struttura
    int matricola;
    //nof_elems dello studente 
    int nof_elems;
    //voto_AdE dello studente  
    int voto_AdE;
    //gruppo di cui lo studente fa parte (NOGROUP oppure indice dell'array group)
    int group;     //CRITICO
};

struct info_group{
    //numero di elementi del gruppo, n_members=0 quando il gruppo non esiste più
    int n_members;  //CRITICO
    //0 false (aperto), 1 true (chiuso)
    int is_closed;  //CRITICO
    //voto massimo del gruppo
    int max_voto;   //CRITICO
};

//struttura che andrà in MEMORIA CONDIVISA
struct info_sim { 
    struct info_student student[POP_SIZE];
    struct info_group group[POP_SIZE];
    int time_left;  //CRITICO
};

#endif
