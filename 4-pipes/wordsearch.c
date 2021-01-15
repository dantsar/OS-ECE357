#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

jmp_buf int_jb;

void sigpipe_handler(){
    fprintf(stderr, "pid %d received signal Broken Pipe\n", getpid());
    longjmp(int_jb, 1);
}

int main(int argc, char **argv)
{
    if(argc != 2){
        fprintf(stderr, "usage %s: input dictionary file\n", argv[0]);
        exit(-1);
    }

    struct sigaction act;
    act.sa_handler = sigpipe_handler;
    if(sigaction(SIGPIPE, &act, NULL) == -1)
        perror("error setting up signal handler\n");

    char *dict_buf = NULL;
    size_t dict_size = 16;
    FILE *dict;
    if( (dict = fopen(argv[1], "r")) == NULL){
        fprintf(stderr, "error opening word list %s\n", argv[1]);
        exit(-1);
    }

    char *in_buf = NULL;
    size_t n = 16; 
    size_t matched = 0;

    while( getline(&in_buf, &n, stdin) != -1)
    {
        if(!setjmp(int_jb)){
            while( (getline(&dict_buf, &dict_size, dict)) != -1){
                if(!strcmp(dict_buf, in_buf)){
                    write(STDOUT_FILENO, in_buf, strlen(in_buf));
                    matched++;
                }
                errno = 0;
            }
            if(errno != 0){
                int errno_save = errno;
                fprintf(stderr, "\n%s: error reading from dictionary input,\n (errno %d) %s\n", argv[0], errno_save, strerror(errno_save));
            }
            
            fseek(dict, 0, 0); /* back to top of dict file */
            errno = 0;
        } else
            break;
    }
    if(errno != 0){
        int errno_save = errno;
        fprintf(stderr, "\n%s: error reading from input,\n (errno %d) %s\n", argv[0], errno_save, strerror(errno_save));
    }

    fprintf(stderr, "Matched %ld words\n", matched);
    return 0;
}
