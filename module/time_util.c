#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "error.h"
#include "../header/time_util.h"

#ifdef NEW_TIMER

timer_t *set_timer(int seconds){
    timer_t id;
    struct sigevent sigev;
    struct itimerspec timer;

    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGALRM;
    timer_create(CLOCK_REALTIME,&sigev,&id);
    
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = 0;
    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_nsec = 0;
    return timer_settime(id,0,&timer,NULL)!=-1 ? id : (void *)-1;
}

int time_left(timer_t *id){
    int result;
    struct itimerspec timer;

    result = timer_gettime(id,&timer);
    return result!=-1 ? timer.it_value.tv_sec : -1;
}
// ########################################################################################
#else

//interfaccia per la creazione di un timer e la visualizzazione del tempo rimanente

//imposta timer
int set_timer(int seconds){
    struct itimerval timer;

    //disabling the periodic timer
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    //setting the timer
    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_usec = 0;

    return setitimer(ITIMER_REAL,&timer,NULL);
}

int time_left(){
    struct itimerval timer;

    //getting the timer value in seconds
    getitimer(ITIMER_REAL,&timer);

    return timer.it_value.tv_sec;
}

#endif


/*

//      PER EVENTUALI TEST

void handler(int sig){
    printf("Tempo scaduto\n");
    //kill process group del chiamante, con segnale sigkill
    killpg(0,SIGKILL);
}

//
//   //   TEST TIMER   //
//

int main(int argc, char* argv[]){
    if(argc!=2){
        printf("Numero di argomenti inseriti non corretto\n");
        exit(EXIT_FAILURE);
    }

    timer_t timer;

    //set del handler
    struct sigaction sa;
    sa.sa_handler = handler;
    sigaction(SIGALRM,&sa,NULL);

    //set del process group id
    setpgid(0,0);
    printf("PARENT(%d): Sono nel process group: %d\n",getpid(), getpgid(0));
    if(!fork()){
        //il figlio si blocca
        printf("CHILD(%d): Sono nel process group: %d\n",getpid(), getpgid(0));
        while(1);
    }

    //il padre crea il timer
    timer = set_timer(atoi(argv[1]));
    if(timer==(void *)-1)
        printf("Errore set_timer\n");
    TEST_ERROR;

    int left;
    //il padre si blocca e conta il tempo rimanente ogni 5 secondi
    while(1){
        left = time_left(timer);
        printf("Tempo rimanente = %d\n", left);
        sleep(5);
    }
}
*/