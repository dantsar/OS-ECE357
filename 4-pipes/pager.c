#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv){

    FILE *input;
    if(argc == 2){
        if((input = fopen(argv[1], "r")) == NULL){
            int errno_save = errno;
            fprintf(stderr, "trouble reading %s\n(errno %d) %s", argv[1], errno_save, strerror(errno_save));
        }
    } else {
        input = stdin;
    }

    int fd_r_tty; /* read tty fd  because blocking */
    int fd_w_tty; /* write tty fd because nonblocking  */

    if((fd_r_tty = open("/dev/tty", O_RDONLY)) == -1) 
        printf("error\n");
    if((fd_w_tty = open("/dev/tty", O_WRONLY|O_NONBLOCK)) == -1) //O_NONBLOCK
        printf("error\n");
    
    char *in_line = NULL;
    size_t in_size = 16;

    int out_count = 0; /* count words that were outputted   */
    size_t bytes;
    while( ( bytes = getline(&in_line, &in_size, input)) != -1) 
    {
        if(out_count == 23)
        {
            char out[] = "--Press RETURN for more--- ";
            if(write(fd_w_tty, out, sizeof out) == -1)
                perror("error writing to tty");
            char buf[2];
            if(read(fd_r_tty, buf, 1) == -1)
                perror("error reading from tty");
            if (*buf == 'q' || *buf == 'Q'){
                fprintf(stderr, "***Pager termninated by Q command***\n");
                exit(0);
            } else if(*buf != '\n'){
                fprintf(stderr, "please input either return or q/Q, I'll just assume you want more...\n");
                if(read(fd_r_tty, buf, 1) == -1)
                    perror("error reading newline"); /* get rid of newline character */
            }
            out_count = 0;
        }
        int written_bytes = 0;
        while((written_bytes += write(fd_w_tty, in_line+written_bytes, bytes-written_bytes)) != bytes); /* hopefully handle partial writes :^) */
        out_count++;
    }
    return 0;
}
