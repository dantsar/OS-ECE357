#include <stdio.h>
#include <stdlib.h>
#include "fifo.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define ITERATIONS 100000
#define WRITERS 8

unsigned long proc_id = -1; /* proc_id of parent  */

int main(int argc, char **argv)
{
    struct fifo *f;

    if((f = mmap(NULL, sizeof(struct fifo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED){  
        print_err("mmap failed (errno %d) %s", errno);                                                          
        exit(-1);                                                                                                       
    }                                                                                                                   

    fifo_init(f);

    /* baby steps: see dir partD for output */
//    switch(fork()){
//        case -1:
//            print_err("error forking (errno %d) %s\n", errno);
//            break;
//        case 0: /* child as writer */
//            for(unsigned long i = 0; i < 100000; i++){
//                fifo_wr(f, i);
//            }
//            break; 
//        default: /* parent as reader */
//            for(unsigned long i = 0; i < 100000; i++){
//                fprintf("reading: %ld\n", fifo_rd(f)); 
//            }
//            break;
//    }

    /* acid test */    
    unsigned long from_writers[WRITERS] = {0};
    unsigned long writer_id;
    for(unsigned long i = 0; i < WRITERS; i++)
    {
        switch(fork()){
            case -1:
                print_err("error forking (errno %d) %s\n", errno);
                break;
            case 0:  /* child as reader  */
                writer_id  = i <<  (8*sizeof(unsigned long) - WRITERS);
                from_writers[proc_id] = writer_id; /* starting point is writer id */

                for(unsigned long j = 0; j < ITERATIONS; j++)
                {
                    unsigned long d = j;
                    d |= writer_id;

                    fifo_wr(f, d);
                }
                exit(0);
                break; 
        }
    }

    /* reader process */
    int writer_index; 
    for(int i = 0; i < (ITERATIONS*WRITERS); i++){
        unsigned long d = fifo_rd(f);
        writer_index = d >> ((sizeof(unsigned long) * 8) - WRITERS);
        if(d != from_writers[writer_index] + 1 && d != 0){
            fprintf(stderr, "mismatch with process id: %d", writer_index);
        }
        from_writers[writer_index] = d;
        fprintf(stderr, "%ld\n", d);
    }

    return 0;
}
