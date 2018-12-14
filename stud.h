#ifndef _STUD_H_
#define _STUD_H_

struct info_student{
    int matricola;  //matricola dello studente associato alla struttura
    int nof_elems;  //nof_elems dello studente
    int voto_AdE;   //voto_AdE dello studente
    int group;     //gruppo di cui lo studente fa parte
}

struct gruppo{
    int members;  //numero di elementi del gruppo
    int is_closed;  //0 se e' chiuso, 1 alrimenti
}

#endif