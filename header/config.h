#ifndef _CONFIG_H_
#define _CONFIG_H_
//file header utile a un po' tutti i file .c

//numero di studenti che saranno parte della simulazione
#define POP_SIZE 25

#define TRUE 1
#define FALSE 0
#define ARG_SIZE 32
#define IPC_KEY ftok("opt.conf",0)

//valori massimi consentiti in opt.conf
#define MAX_VALUE 20
#define TIME_LIMIT 120

//definire soltanto se si vuole che si stampino dei check
#define DEBUG

//definire soltanto se si vuole che con una TEST_ERROR si esca in caso di errore
#define EXIT_ON_ERROR

#endif
