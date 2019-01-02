#define MSG_LEN 100

struct msgbuf{
   int mtype;             /* message type, sarà > 0 */
   char testo[MSG_LEN];    /* message testo */
};
