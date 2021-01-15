#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "sem.h"
#include "tas.h"
#include "spinlock.h"

int proc_num = -1; /* parent id */

int main(int argc, char **argv)
{
    struct sem *t_sem;
    if((t_sem = mmap(NULL, sizeof(struct sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED){
        fprintf(stderr, "mmap failed: (errno %d) %s\n", errno, strerror(errno));
        exit(-1);
    }
    
    sem_init(t_sem, 420);

    int times = 1000000;

    for(int i = 0; i < N_PROC; i++){
        switch(fork()){
            case -1:
                fprintf(stderr, "error forking (errno %d) %s\n", errno, strerror(errno));
                break;
            case 0: /* child  */
                proc_num = i;
                i = N_PROC; 
                
                for(int j = 0; j < times; j++){
                    sem_inc(t_sem);
                }

                return -1;
                break;
            default: /* parent */
                break;
        }

    }

    for(int j = 0; j < N_PROC*times; j++){
        sem_wait(t_sem);
    }

    printf("-----\n");
    printf("%d\n", t_sem->num);

    return 0;
}
