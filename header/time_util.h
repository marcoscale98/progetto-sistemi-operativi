#ifndef _TIME_UTIL_H_
#define _TIME_UTIL_H_

#define NEW_TIMER

#ifdef NEW_TIMER

timer_t *set_timer(int seconds);
int time_left(timer_t *timer);

#else

int set_timer(int seconds);
int time_left();

#endif



#endif