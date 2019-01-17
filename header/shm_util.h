#ifndef _SHM_UTIL_H_
#define _SHM_UTIL_H_

#include "config.h"

#define SHM_SIZE sizeof(struct info_sim)

struct info_student{
    int matricola;  //matricola dello studente associato alla struttura
    int nof_elems;  //nof_elems dello studente
    int voto_AdE;   //voto_AdE dello studente
    int group;     //gruppo di cui lo studente fa parte (NOGROUP oppure indice dell'array group)
};

struct info_group{
    int number;  //numero di elementi del gruppo
    int is_closed;  //0 false (aperto), 1 true (chiuso)
    int max_voto;   //voto massimo del gruppo
};

//struttura che andrà in MEMORIA CONDIVISA
struct info_sim { 
    struct info_student student[POP_SIZE];
    struct info_group group[POP_SIZE]; //EMPTY=vuoto
    int time_left;
};

#endif
