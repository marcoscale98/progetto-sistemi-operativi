#ifndef _ERROR_H_
#define _ERROR_H_

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

#ifdef EXIT_ON_ERROR
#define TEST_ERROR    if (errno) {fprintf(stderr, \
                       "%s:%d: PID=%5d: Error %d (%s)\n",\
                       __FILE__,\
                       __LINE__,\
                       getpid(),\
                       errno,\
                       strerror(errno)); \
                       exit(EXIT_FAILURE);errno=0;}
#else
#define TEST_ERROR    if (errno) {fprintf(stderr, \
                       "%s:%d: PID=%5d: Error %d (%s)\n",\
                       __FILE__,\
                       __LINE__,\
                       getpid(),\
                       errno,\
                       strerror(errno));errno=0;}
#endif
#endif
