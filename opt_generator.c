#include <stdio.h>
#include "header/error.h"
#include "header/conf_reader.h"

// SERVE A NOI, NON NECESSARIAMENTE LO INCLUDEREMO NEL PROGETTO

int main(){
    FILE *fd = fopen("opt.conf","r+");
    TEST_ERROR;

    srand(getpid());
    int p2 = rand()%1000+1,
        p3 = rand()%1000+1,
        p4 = rand()%1000+1,
        prob_2 = p2*100/(p2+p3+p4),
        prob_3 = p3*100/(p2+p3+p4),
        prob_4 = 100-prob_2-prob_3;
    fprintf(fd,"sim_time %d\n",rand()%(TIME_LIMIT-9)+10);
    fprintf(fd,"prob_2 %d\n",prob_2);
    fprintf(fd,"prob_3 %d\n",prob_3);
    fprintf(fd,"prob_4 %d\n",prob_4);
    fprintf(fd,"nof_invites %d\n",rand()%MAX_VALUE+1);
    fprintf(fd,"max_reject %d\n", rand()%MAX_VALUE+1);

    printf("Generati dei parametri casuali per opt.conf\n");

    fclose(fd);
    TEST_ERROR;
}