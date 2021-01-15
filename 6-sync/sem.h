#ifndef __SEM_H
#define __SEM_H

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define N_PROC 64 /* number of simulated processes */

struct sem{
    int num;
    int wait_list[N_PROC]; 
    char lock; 
};

/* prints error messages  */
void print_err(const char *msg, int errno_save);

/*  initalize the semaphore it count and its data structures */
void sem_init(struct sem *s, int count);

/*  attempt the "P" operation (atomically decrement the semaphore)
* return 0 if the operation would block  (is s-> num == 0)
* return 1 otherwise */
int sem_try(struct sem *s);

/*  Perform the P operation, block until successful  */
void sem_wait(struct sem *s);

/* increment num and wakeup all processes if num becomes positive */
void sem_inc(struct sem *s);

#endif
