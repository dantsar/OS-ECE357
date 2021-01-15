#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#define BUFFER_SIZE 4096
char buff[BUFFER_SIZE];

void err_n_die(const char *, const char *); 
void concat(const char *, int, int);

int main(int argc, char **argv)
{
    int fd_infile = 0;
    int fd_outfile = 1;  
    char *file = NULL;

    if(argc == 1)
        concat("-", fd_infile, fd_outfile);

    int index = 1;      
    if(argc >= 3 && !strcmp(argv[1], "-o")){
        if((fd_outfile = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) //else read from specified file
            err_n_die("***ERROR*** can not open (%s) for writing\n", argv[index+1]);

        if(argc == 3)
            concat("-", fd_infile, fd_outfile);
        index = 3;
    }        
    
    for(index = index; index < argc; index++)
    {
        if(!strcmp(argv[index], "-")) // "-" means stdin
            fd_infile = 0;
        else if((fd_infile = open(argv[index], O_RDONLY)) == -1) //else read from specified file
            err_n_die("***ERROR*** can not read from  (%s)\n", argv[index]);

        concat(argv[index], fd_infile, fd_outfile);

        if(fd_infile != STDIN_FILENO)
            if(close(fd_infile) == -1) 
                err_n_die("***ERROR*** can not close (%s)\n", argv[index]);
    }

    if(close(fd_outfile))
        err_n_die("***ERROR*** can not close (%s)\n", argv[index]);

    return 0;
}

void concat(const char *cur_file, int fd_infile, int fd_outfile)
{
    ssize_t tw_bytes = 0, rbytes = 0;    /* total written and read bytes */
    int is_binary_file = 0, r_w_calls = 0;

    while((rbytes = read(fd_infile, buff, BUFFER_SIZE)) > 0)
    {
        r_w_calls++;
        ssize_t wbytes = 0;
        tw_bytes = 0;

        // check if input is from a binary file
        for(int i = 0; !is_binary_file && i < rbytes; i++)
            if( !(isprint(buff[i]) || isspace(buff[i])) )
                is_binary_file = 1;

        do{
            if((wbytes = write(fd_outfile, buff + tw_bytes, rbytes - tw_bytes)) == -1)
                err_n_die("***ERROR*** writing to output file\n", "");
            r_w_calls++;
	        tw_bytes += wbytes;
            rbytes -= wbytes;
        }while(rbytes > 0); //handles partial writes

    }

    if(!strcmp(cur_file, "-"))
        fprintf(stderr, "<standard input>:");
    else
        fprintf(stderr, "%s:", cur_file);

    if(is_binary_file)
        fprintf(stderr, "  ***WARNING*** binary input detected");
    fprintf(stderr, "\n\tbytes: %ld\n\tRead/Write Calls: %d\n", tw_bytes, r_w_calls);
}

//reports errors and exits the process
void err_n_die(const char *err_msg, const char *prob_file)
{
    int errno_save = errno;
    fprintf(stderr, err_msg, prob_file);
    if(errno_save != 0)
        fprintf(stderr, "(errno: %d) %s\n", errno_save, strerror(errno_save));

    exit(-1);
}
