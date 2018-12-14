#ifndef _ERROR_H_
#define _ERROR_H_

#define TEST_ERROR    if (errno) {fprintf(stderr, \
                       "%s:%d: PID=%5d: Error %d (%s)\n",\
                       __FILE__,\
                       __LINE__,\
                       getpid(),\
                       errno,\
                       strerror(errno)); exit(EXIT_FAILURE);}

#endif