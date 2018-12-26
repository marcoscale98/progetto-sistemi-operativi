#ifndef _SHM_UTIL_H_
#define _SHM_UTIL_H_

#define NOGROUP -1
#define SHM_SIZE sizeof(struct classroom)

struct info_student{
    int matricola;  //matricola dello studente associato alla struttura
    int nof_elems;  //nof_elems dello studente
    int voto_AdE;   //voto_AdE dello studente
    int group;     //gruppo di cui lo studente fa parte
};

struct info_group{
    int n_members;  //numero di elementi del gruppo
    int is_closed;  //0 false (aperto), 1 true (chiuso)
    int pref_nof_elems;     //moda del valore nof_elems dei membri
    int max_voto;   //voto massimo del gruppo
};

//struttura che andrà in MEMORIA CONDIVISA
struct info_sim { 
    struct info_student student[POP_SIZE];
    struct info_group group[POP_SIZE];
    int time_left;
};

#endif
