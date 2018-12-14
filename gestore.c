#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "header/error.h"

#define POP_SIZE 3

int main(){
    int i, value;
    char matricola[5]={'\0'},*args[] = {"    ",NULL};
    
    for(i=0;value && i<POP_SIZE;i++){
        switch(value = fork()){
            case -1:
                TEST_ERROR;
                break;
            case 0:
                sprintf(matricola,"%d",i);
                execlp("./student","student",matricola,NULL);
                TEST_ERROR;
                break;
            default:
                break;
        }    
    }

    //semaforo inizializzato a -POP_SIZE
} 