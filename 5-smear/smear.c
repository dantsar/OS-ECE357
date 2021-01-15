#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void err_print(const char *msg, const char *str){
    int errno_save =  errno;
    fprintf(stderr, msg, str);
    if(errno_save != 0)
        fprintf(stderr, "(errno %d): %s\n", errno_save, strerror(errno_save));
}

int main(int argc, char **argv)
{
    if(argc < 4){
        fprintf(stderr, "%s usage: %s TARGET REPLACEMENT file1 {file2 ...}\n", argv[0], argv[0]);
        exit(-1);
    }else if (strlen(argv[1]) != strlen(argv[2])){
        fprintf(stderr, "TARGET str should be the same length as REPLACEMENT\n");
        exit(-1);
    }

    int fd;
    struct stat info;
    char *targ = argv[1];
    char *rep  = argv[2];

    for(int i = 3; i < argc; i++){
        if((fd = open(argv[i], O_RDWR)) == -1){
            err_print("error opening %s\n", argv[i]);
            continue;
        }

        if(fstat(fd, &info) == -1){
            err_print("error getting stats for %s\n", argv[i]);
            continue;
        }

        int size = info.st_size;
        if(size == 0){
            err_print("empty file: %s\n",argv[i]);
            continue;
        }
       
        char *file;
        if( (file = mmap(NULL, size+1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){ /* size+1 to add null character at the end */
            err_print("error mapping file %s\n", argv[i]);
            continue;
        }

        file[size] = '\0';  /* to make the file null terminating for strstr */
        char *pattern_index;

        while(pattern_index = strstr(file, targ)){ /* while pattern is still in file  */
            for(int i = 0; rep[i] != '\0'; i++){
                pattern_index[i] = rep[i];
            }
        }

        if(munmap(file, size) == -1)
            err_print("error unmapping file %s\n", argv[i]);
    }
    return 0;
}
