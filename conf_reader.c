#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "header/error.h"
#define POPSIZE 10

struct student{
    int nof_elems;
    int max_rejects;
    int nof_invites;
};

void init_student(char*,struct student*);

int main(){
    struct student stud;
    init_student("opt.conf",&stud);
    printf("nof_elems: %d\nmax_rejects: %d\nnof_invites: %d\n",stud.nof_elems, stud.max_rejects, stud.nof_invites);

}

void init_student(char* path, struct student* stud){
    FILE* fs = fopen(path, "r");
    char key[10];
    int value;
    float prob_2, prob_3, prob_4;

    //legge chiavi e valori
    while(fscanf(fs,"%s %d",key,&value)!=EOF){
        if(!strcmp(key,"max_rejects"))
            stud->max_rejects = value;
        else if(!strcmp(key,"nof_invites"))
            stud->nof_invites = value;
        else if(!strcmp(key,"prob_2"))
            prob_2 = value/100;
        else if(!strcmp(key,"prob_3"))
            prob_3 = value/100;
        else if(!strcmp(key,"prob_4"))
            prob_4 = value/100;
    }

    //inizializzazione nof_elems
    srand(getpid());
    int val = rand()%POPSIZE;
    printf("Val: %d\n", val);
    if(val<POPSIZE*prob_2)
        stud->nof_elems = 2;
    else if(val<POPSIZE*prob_3)
        stud->nof_elems = 3;
    else if(val<POPSIZE*prob_4)
        stud->nof_elems = 4;

    fclose(fs);
}