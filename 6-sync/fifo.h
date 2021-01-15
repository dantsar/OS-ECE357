#ifndef __FIFO_H
#define __FIFO_H

#include "sem.h"

#define MYFIFO_BUFSIZE 4096

struct fifo{
    unsigned long mem[MYFIFO_BUFSIZE];
    int next_write, next_read;
    struct sem full, empty;
    struct sem mutex;
};

/* initalize the shared memory fifo *f including any required 
 * underlying initializations (such as sem_init)
 * fifo will have length of MYFIFO_BUFSIZE elements (4k)  */
void fifo_init(struct fifo *f);

/* Enqueue the data word d into the FIFO, blocking 
 * unless and until the FIFO has room to accept it
 * Using the semaphore primitives for blocking and waking
 * Writing to the FIFO shall cause any and all processes 
 * that have been blocked on the "empty" condition to wake up */
void fifo_wr(struct fifo *f, unsigned long d);


/* Dequeue the next data word from the FIFO and return it.
 * Blocks until there are available words queued in the FIFO
 * Reading from the FIFO shall cause all processes that have been 
 * blocked on the "full" condition to wake up*/
unsigned long fifo_rd(struct fifo *f);

#endif
