#include <sys/time.h>
#include <stdlib.h>

//interfaccia per la creazione di un timer e la visualizzazione del tempo rimanente

//ATTENZIONE: il timer non e' ereditato tramite la fork

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