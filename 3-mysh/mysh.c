#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
 
#define PATH_MAX 4096
#define CMD_BUFF 4096

int pwd(int);
int cd (int, char *);
void print_err(const char *, const char *);

int parse_cmd(char *, char **, size_t *, int*, int *);
int redirect(int *, mode_t, const char *, const char *, const char *);

int main(int argc, char **argv, char **envp)
{
    int fd = 0; 
    if(argc == 2){
        if((fd = open(argv[1], O_RDONLY)) == -1){
            print_err("error with reading script: %s\n", argv[1]);
            return -1;
        }
    }else if (argc > 2){
        print_err(" %s incorrect usage\n", argv[0]);
        return -1;
    }

    size_t cmd_size = 16;
    char **cmd = calloc(sizeof(char*), cmd_size); /* will be adjusted if needed */
    char *line = NULL;
    int redir[3] = {-1, -1, -1}; /* 0:stdin 1:stdout 2:stderr */

    char *cmd_line = NULL;
    size_t buf_size = CMD_BUFF;
    FILE *fd_std = fdopen(fd, "r"); /* using stdlib */

    int ret_code; /* return code of commands */
    char str[16]; /* str for int */
    while( getline(&line, &buf_size, fd_std) != -1)
    {
        if(*line == '#' || isspace(*line)) continue; /* getline removes whilespaces, so check first char for # */

        /* replace newline with null */
        int i;
        for(i = 0; *(line+i) != '\n'; i++);
        *(line+i) = '\0';

        /* parse_cmd */
        int new_argc = 1;
        if(( ret_code = parse_cmd(line, cmd, &cmd_size, &new_argc, redir) == 1)) continue;

        if(!strcmp(cmd[0], "cd")){
            if(new_argc == 1){
                char home[2] = "~";
                ret_code = cd(new_argc, home);
            }else
                ret_code = cd(new_argc, cmd[2]);
        }else if(!strcmp(cmd[0], "pwd")){
            ret_code = pwd(new_argc);
        }else if(!strcmp(cmd[0], "exit")){
            if(new_argc == 1)
                exit(ret_code);
            else
                exit(atoi(cmd[2]));
        }else{
            pid_t pid;
            int status; 
            struct rusage ru; 
            struct timeval start, end, total; /* needed to get real time of program */

            gettimeofday(&start, NULL);
            switch(pid = fork()){
                case -1:
                    print_err("error forking", NULL);
                    break;
                case 0:
                    for(i = 0; i < 3; i++){
                        if(redir[i] != -1){
                            if(dup2(redir[i], i) < 0){
                                print_err("Error redirecting child\n", NULL);
                                exit(1);
                            }
                            close(redir[i]);
                        }
                    }

                    if(fd != 0)
                        fclose(fd_std);
                    execvp(cmd[0], cmd); /* execute the command! */

                    print_err("error executing %s\n", cmd[0]);
                    exit(127);
                    break;
                default:
                    if(wait3(&status, 0, &ru) == -1){
                        sprintf(str, "%d", pid);
                        print_err("wait for child %s failed\n", str);
                        continue;
                    }

                    gettimeofday(&end, NULL);
                    timersub(&end, &start, &total);
                    fprintf(stderr, "Child Process: %d ", pid);
                    if(status != 0){
                        if(WIFSIGNALED(status)){
                            fprintf(stderr,"Exited with signal %d (%s)\n", WTERMSIG(status), strsignal(WTERMSIG(status)));
                            ret_code = WTERMSIG(status);
                            if(WCOREDUMP(status)) ret_code += 128;
                        }else{
                            fprintf(stderr,"Exited with return value %d\n", WEXITSTATUS(status));
                            ret_code = WEXITSTATUS(status);
                        }
                    } else{
                        print_err("Exited normally\n", NULL);
                        ret_code = 0;
                    }
                    fprintf(stderr, "Real: %ld.%.6lds ", total.tv_sec, total.tv_usec);
                    fprintf(stderr, "User: %ld.%.6lds ", ru.ru_utime.tv_sec, ru.ru_utime.tv_usec);
                    fprintf(stderr, "Sys: %ld.%.6lds\n", ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
                    
                    for(i = 0; i < 3; i++){
                        close(redir[i]);
                        redir[i] = -1;
                    }
                    break;
            }
        }
        errno = 0;
    }
    if(errno != 0)
        print_err("%s err\n", "getline");

    fclose(fd_std);
    free(cmd);
  
    sprintf(str, "%d", ret_code);
    print_err("end of file read, exiting shell with exit code %s\n", str);
    exit(ret_code);
}

int parse_cmd(char *line, char **cmd, size_t *cmd_size, int *new_argc, int *redir) 
{ 
    *new_argc = 1;
    int index = 1; /* starting index of adding parameters */
    char *tok;
    cmd[0] = strtok(line, " ");

    while(tok = strtok(NULL, " "))
    {
        int ret = 1; 
        if(strstr(tok,"<")){
            redir[0] = 1;
            ret = redirect(redir, O_RDONLY, tok+1, NULL, NULL);
        }else if(strstr(tok, "2>>")){
            redir[2] = 1;
            ret = redirect(redir, O_CREAT|O_APPEND|O_WRONLY, NULL, NULL, tok+3);
        }else if(strstr(tok, ">>")){
            redir[1] = 1;
            ret = redirect(redir, O_CREAT|O_APPEND|O_WRONLY, NULL, tok+2, NULL);
        }else if(strstr(tok, "2>")){
            redir[2] = 1;
            ret = redirect(redir, O_CREAT|O_TRUNC|O_WRONLY, NULL, NULL, tok+2);
        }else if(strstr(tok, ">")){
            redir[1] = 1;
            ret = redirect(redir, O_CREAT|O_TRUNC|O_WRONLY, NULL, tok+1, NULL);
        }

        if(ret == 0){
            continue;
        }else if(ret == -1){
            errno = 0;
            return 1;
        }

        if(*cmd_size <= index+1){
            *cmd_size *= 2;
            cmd = realloc(cmd, *cmd_size);
        }
        cmd[index++] = tok;
        *new_argc += 1;
    }

    cmd[index] = NULL;
    return 0;
}

int redirect(int *redir, mode_t mode, const char *redir_in, const char *redir_out, const char *redir_err)
{
    if(redir[0] == 1){
        if((redir[0] = open(redir_in, mode)) == -1){
            print_err("error reading input file: %s\n", redir_in);
            return -1;
        }
    }
    if(redir[1] == 1){
        if((redir[1] = open(redir_out, mode, 0666)) == -1){
            print_err("error redirecting stdout to %s\n", redir_out);
            return -1;
        }
    }
    if(redir[2] == 1){
        if((redir[2] = open(redir_err, mode, 0666)) == -1){
            print_err("error redirecting stderr to %s\n", redir_err);
            return -1;
        }
    }
    return 0;
}

int cd(int argc, char *dir){
    if(argc > 2){
        print_err("%s: too many arguments\n", "cd");
        return 127;
    }
    char new_dir[PATH_MAX];
    bzero(new_dir, PATH_MAX);
    struct passwd *pw = getpwuid(getuid()); /* https://www.stev.org/post/cgethomedirlocationinlinux  */

    if(*dir == '~'){
        strcat(new_dir, pw->pw_dir);
        *dir = '/';
        strcat(new_dir, dir);
    }else if(argc == 1)
        strcat(new_dir, pw->pw_dir);
    else
        strcat(new_dir, dir);
    
    if(chdir(new_dir) == -1){
        print_err("%s: error changing directories\n", "cd");
        return 127;
    }

    return 0;
}

int pwd(int argc)
{
    if(argc != 1){
        print_err("%s: too many arguments\n", "pwd");
        return 127;
    }
    char cwd[PATH_MAX]; /* current working directory */
    bzero(cwd, PATH_MAX);
    if(getcwd(cwd, PATH_MAX) == NULL)
        print_err("%s err\n", "pwd");
    printf("%s\n", cwd);
    return 0;
}

void print_err(const char *msg, const char *cmd)
{
    int errno_save = errno; 
    fprintf(stderr, msg, cmd);
    if(errno_save != 0)
        fprintf(stderr,"(errno %d) %s\n", errno_save, strerror(errno_save));
    return; 
}
