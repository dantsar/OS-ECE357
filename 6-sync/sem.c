#include "sem.h"
#include "spinlock.h"

static void nice_handler(){  
    /* performs to critical task of not killing itself :^) */
}

void print_err(const char *msg, int errno_save){
    fprintf(stderr, msg, errno_save, strerror(errno_save));
}

void sem_init(struct sem *s, int count)
{
    s->num = count;
    s->lock = 0;
    memset(s->wait_list, 0, N_PROC); 
    signal(SIGUSR1, nice_handler); /* SIGUSR1's default action terminates... that's bad */
}

int sem_try(struct sem *s)
{
    spin_lock(&(s->lock));

    if(s->num < 1){
        spin_unlock(&(s->lock));
        return 0;
    } else {
        s->num -= 1;
        spin_unlock(&(s->lock));
        return 1;
    }
}

void sem_wait(struct sem *s)
{
    sigset_t set;
    for(;;)
    {   /* block SIGUSR1 */
        spin_lock(&(s->lock));

        if(s->num > 0){
            s->num -= 1;
            spin_unlock(&(s->lock));
            return;
        }

        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        if(sigprocmask(SIG_BLOCK, &set, NULL) == -1)
            print_err("sem_wait: sigprocmask error: (errno %d) %s\n", errno);

        int i;
        for(i = 0; s->wait_list[i] != 0; i++); /* put process on waitlist first available */
        s->wait_list[i] = getpid();

        spin_unlock(&(s->lock));

        signal(SIGUSR1, nice_handler); /* SIGUSR1's default action terminates... that's bad */
        sigemptyset(&set); /* mask now doesn't block SIGUSR1 */
        sigsuspend(&set);
        if(errno != EINTR)
            print_err("sem_wait sigsuspend error: (errno %d) %s\n", errno);
    }

}

void sem_inc(struct sem *s)
{
    spin_lock(&(s->lock));
    s->num++;

    if(s->num == 1) /* wakey wakey */
    {   
        for(int i = 0; i < N_PROC; i++){
            if(s->wait_list[i] != 0){
                /* send signal  */
                if(kill(s->wait_list[i], SIGUSR1) == -1)
                    print_err("sem_inc: kill error: (errno %d) %s", errno);
                s->wait_list[i] = 0;
            }
        }
    }

    spin_unlock(&(s->lock));
}
