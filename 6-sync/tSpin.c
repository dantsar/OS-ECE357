#include "spinlock.h"
#include "tas.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#define N_PROC 64

int my_procnum = 0; /* 0 for parent process */

struct Num{
    long long num; 
    char lock;
};

int main()
{
    /* shared memory for test */
    int errno_save;
    struct Num *test; 
    if((test = mmap(NULL, sizeof(struct Num), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED){
        errno_save = errno;
        fprintf(stderr, "mmap failed: (errno %d) %s\n", errno_save, strerror(errno_save));
        exit(-1);
    }

    /* initalize struct */
    test->num = 0;
    test->lock = 0;

    spin_lock(&(test->lock));
    spin_unlock(&(test->lock));

    for(int i = 0; i < N_PROC; i++)
    {
        switch(fork()){
            case 0:             /* child */
                my_procnum = i;
                i = N_PROC;     /* escape for loop */
//                fprintf(stderr, "created process %d\n", my_procnum);
                break;
            case -1:            /* uh oh */
                errno_save = errno;
                fprintf(stderr, "fork failed: (errno %d) %s\n", errno_save, strerror(errno_save));
                exit(-1);
                break;
            default:            /* parent */
                break;
        } 

    }

    volatile int old_num = test->num;

    // for(;;){

    //     spin_lock(&(test->lock));

    //     old_num = test->num;
    //     old_num++;
    //     test->num = old_num;
    //     if(old_num != test->num){
    //         fprintf(stderr, "mismatch in process %d\n", my_procnum);
    //     } else{
    //         fprintf(stderr, "num %lld\n", test->num);
    //     }

    //     spin_unlock(&(test->lock));
    // }

   for(;;){
       while(test->lock != 0)
           sched_yield();

       test->lock = 1;

       old_num = test->num;
       old_num++;
       test->num = old_num;
       
       if(old_num != test->num){
           fprintf(stderr, "mismatch in process %d\n", my_procnum);
           exit(-1);
       } else{
           fprintf(stderr, "num %lld\n", test->num);
       }

       test->lock = 0;
   }

    /* never really gets here :^) */
    if(munmap(test, sizeof(struct Num) == -1)){
        errno_save = errno;
        fprintf(stderr, "fork failed: (errno %d) %s\n", errno_save, strerror(errno_save));
        exit(-1);
    }
    return 0;
}
