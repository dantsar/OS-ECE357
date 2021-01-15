#include "fifo.h"
#include "sem.h"

void fifo_init(struct fifo *f)
{
    sem_init(&(f->empty), MYFIFO_BUFSIZE);
    sem_init(&(f->full), 0);
    sem_init(&(f->mutex), 1);
    f->next_read = 0;
    f->next_write = 0;
}

void fifo_wr(struct fifo *f, unsigned long d)
{
    for(;;) /* loop until written  */
    { 
        sem_wait(&(f->empty)); /* wait until not full */

        if(sem_try(&(f->mutex)))
        {
            f->mem[f->next_write] = d;
            f->next_write = (f->next_write + 1) % MYFIFO_BUFSIZE; /* circ buf  */
            sem_inc(&(f->mutex)); /* release lock */
            sem_inc(&(f->full)); /* wake up waiting readers */
            return;
        }
        sem_inc(&(f->empty)); 
    }
}

unsigned long fifo_rd(struct fifo *f)
{
    unsigned long d;
    for(;;) /* loop until read  */
    {
        sem_wait(&(f->full)); /* wait until not empty */

        if(sem_try(&(f->mutex)))
        {
            d = f->mem[f->next_read];
            f->next_read = (f->next_read + 1) % MYFIFO_BUFSIZE; /* circ buf  */
            sem_inc(&(f->mutex)); /* release lock & wakeup others */
            sem_inc(&(f->empty)); /* wake up waiting writers*/
            return d;
        }
        sem_inc(&(f->full)); /* relese lock, we failed :^(  */
    }
}
