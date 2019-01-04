#ifndef _MSG_UTIL_H_
#define _MSG_UTIL_H_

#define MSG_LEN 100

struct msgbuf{
   int mytype;             /* message type, sarà > 0 */
   char text[MSG_LEN];    /* message testo */
};

#endif
