#ifndef _STUD_H_
#define _STUD_H_

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

struct student{
    int nof_elems;
    int max_reject;
    int nof_invites;
};

// struct info_group gruppo[POP_SIZE];

#endif