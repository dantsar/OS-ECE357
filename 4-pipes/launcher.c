#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void close_pipe(int *, const char *);

int main(int argc, char **argv){

    int fd_pipe1[2];
    int fd_pipe2[2];
    char *wordgen_input;
    int children[3] = {0};    /* to hold children's pid and exit status */
    int status[3]   = {0};

    if(argc == 2)
        wordgen_input = argv[1];
    else if (argc == 1) 
        wordgen_input = NULL;
    else{
        fprintf(stderr, "too many inputs");
        exit(-1);
    }

    if(pipe(fd_pipe1) == -1) 
        perror("could not create first pipe");

    switch(children[0] = fork()){
        case 0:
            if(dup2(fd_pipe1[1], STDOUT_FILENO) < 0) /* dup stdout to read of pipe1 */
                perror("error duping stdout of gen\n");
            close_pipe(fd_pipe1, "wordgen");

            execlp("./wordgen", "wordgen", wordgen_input, NULL);
            perror("error: could not start wordgen...Does it exist?\n");
            exit(-1);
            break;
        case -1:
            perror("error forking to wordgen\n");
            exit(-1);
            break;
    }

    if( pipe(fd_pipe2) == -1)
        perror("could not create pipe2");

    switch(children[1] = fork()){
        case 0:
            if(dup2(fd_pipe1[0], STDIN_FILENO) < 0) /* dup stdout to read of pipe1 */
                perror("problem redirecting wordsearch's input\n");
            if(dup2(fd_pipe2[1], STDOUT_FILENO) < 0)
                perror("problem redirecting wordsearch's output\n");
            close_pipe(fd_pipe1, "./wordsearch"); 
            close_pipe(fd_pipe2, "./wordsearch"); 
            
            execlp("./wordsearch", "./wordsearch", "words.txt", NULL);
            fprintf(stderr, "could not start wordsearch, Does it exist?\n");
            exit(-1);
            break;
        case -1:
            perror("error forking to wordsearch\n");
            exit(-1);
            break;
    }

    switch(children[2] = fork()){
        case 0:
            if(dup2(fd_pipe2[0], STDIN_FILENO) < 0)
                perror("problem redirecting pager's input\n");
            close_pipe(fd_pipe1, "./pager");
            close_pipe(fd_pipe2, "./pager");

            execlp("./pager", "./pager", NULL);
            perror("could not launch pager");
            exit(-1);
            break;
        case -1:
            perror("error forking to pager\n");
            exit(-1);
            break;
    }

    close_pipe(fd_pipe1, "./launcher");
    close_pipe(fd_pipe2, "./launcher");

    for(int i = 2; i >= 0; i--){
        if(waitpid(children[i], &status[i], 0) == -1){
            fprintf(stderr, "wait for child %d failed %s", children[i], strerror(errno));
        }
        fprintf(stderr, "Child %d exited with %d\n", children[i], status[i]);
    }
    return 0;
}

void close_pipe(int pipe[], const char *proc)
{
    if(close(pipe[0]) == -1){
        fprintf(stderr, "%s: error closing read side of pipe\n (errno %d) %s\n", proc, errno, strerror(errno));
    }
    if(close(pipe[1]) == -1){
        fprintf(stderr, "%s: error closing read side of pipe\n (errno %d) %s\n", proc, errno, strerror(errno));
    }
}
