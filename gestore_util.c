#include "../header/gestore_util.h"
#include <stdio.h>

//stampa per ogni voto il numero di studenti che ha tale voto
void print_data(int array[], int size){
    if(array && size>0){
        printf("VOTO\tFREQUENZA\n");

        int v, i,cnt;
        for(v=0;v<30;v++){
            for(i=0, cnt=0;i<size;i++){
                if(v==array[i])
                    cnt++;
            }
            if(cnt>0){
                printf("%d\t%d\n",v,cnt);
            }
        }
    }
}