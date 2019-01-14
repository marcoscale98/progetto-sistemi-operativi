#ifndef _CONFIG_H_
#define _CONFIG_H_

#define POP_SIZE 50
#define TRUE 1
#define FALSE 0
#define ARG_SIZE 32
#define IPC_KEY ftok("opt.conf",0)
#define CRITIC_TIME (0.2*POP_SIZE)

//alternare la strategia
//#define STRATEGIA_NUOVA

//definire soltanto se si vuole che si stampino dei check
//#define DEBUG

//definire soltanto se si vuole che con una TEST_ERROR si esca in caso di errore
//#define EXIT_ON_ERROR

#endif
