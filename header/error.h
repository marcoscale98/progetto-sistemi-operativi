#ifndef _ERROR_H_
#define _ERROR_H_

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#define TEST_ERROR    if (errno) {fprintf(stderr, \
                       "%s:%d: PID=%5d: Error %d (%s)\n",\
                       __FILE__,\
                       __LINE__,\
                       getpid(),\
                       errno,\
                       strerror(errno)); exit(EXIT_FAILURE);}

#endif
