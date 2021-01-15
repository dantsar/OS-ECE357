#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#define INODE_BUFF 4096 //Buffer size to hold nondir ino w/ links > 1

void rec_exp_fs(const char *);
void get_stats(const char *, const char *, struct stat);
void print_err(const char *, const char *);
int check_inodes(unsigned int);
void ino_type(struct stat);
void acc_reg_size(struct stat);

static size_t tot_size;
static size_t disk_blocks;
static size_t ino_g1;   //inodes with links greater than 1
static size_t syms_to_nowhere; 
static size_t problematic;
static size_t inodes;
static size_t all_inodes[7]; // 0-7 dir, chr, blk, reg, fifo, lnk, sock

int main(int argc, char **argv){
    if(argc != 2){ 
        printf("usage: %s <pathname>\n", argv[0]); 
        exit(-1);
    }

    rec_exp_fs(argv[1]);  

    printf("\nTotal Inodes: %16ld\n", inodes);
    const char type[][8] = {"DIR", "CHR", "BLK", "REG", "FIFO", "SYMLINK", "SOCKET"};
    for(int i = 0; i < 7; i++)
        printf("%12s: %lu\n", type[i], all_inodes[i]);
    printf("Reg File Size: %15ld\n", tot_size);
    printf("Disk Blocks: %17ld\n", disk_blocks);
    printf("File Size:Disk Blocks %8.3f\n", (float) tot_size/disk_blocks);
    printf("Inodes With link > 1: %8ld\n", ino_g1);
    printf("Sym Links to nowhere: %8ld\n", syms_to_nowhere);
    printf("Problematic filenames: %7ld\n", problematic);
    return 0;
}

void rec_exp_fs(const char *pathname) //recursively explore filesytem
{ 
    struct stat statbuf;
    if(lstat(pathname, &statbuf) == -1){
        print_err("***ERROR*** Trouble getting stat for %s\n", pathname);
        if(errno == ENOENT)
            return;
    }

    DIR *dir = opendir(pathname);
    if(dir == NULL && errno != 0){
        print_err("***ERROR*** PROBLEM OPENING DIRECTORY: %s\n", pathname);
        return;
    }
    
    errno = 0;
    struct dirent *dirp;
    while( (dirp = readdir(dir)) )
    {
        char new_path[PATH_MAX];
        snprintf(new_path, PATH_MAX, "%s/%s", pathname, dirp->d_name); //create a pathname to the dirent

        if(lstat(new_path, &statbuf) == -1){
            print_err("***ERROR*** Trouble getting stat for %s\n", pathname);
            continue;
        }

        if(errno == ENOENT){    
            print_err("***ERROR*** %s DOES NOT EXIST\n", dirp->d_name);
            continue;
        } else if(errno == EACCES){
            print_err("***ERROR*** PERMISSION DENIED TO %s || INODE STILL COUNTED\n", new_path);
            get_stats(new_path, dirp->d_name, statbuf);
            break;
        }
            
        if(strcmp(dirp->d_name, "..") && strcmp(dirp->d_name, ".")) //ignore . and ..
        {   
            get_stats(new_path, dirp->d_name, statbuf);
            if((statbuf.st_ino == 1 || statbuf.st_ino == 2))
                print_err("**WARNING** MOUNTPOINT DETECTED: %s\n", new_path);
            else if(dirp->d_type == DT_DIR) // if dirent is a dir and not a mount point explore the direct
            {
                rec_exp_fs(new_path);
            }
        }
        errno = 0;
    }
    if(dirp == 0 && errno != 0)
        print_err("***ERROR*** problem reading entry %s\n", dirp->d_name);

    closedir(dir);
}

void get_stats(const char *pathname, const char *name, struct stat statbuf)
{
    if( (statbuf.st_mode & S_IFMT) == S_IFLNK ){
        if( access(pathname, F_OK) == -1 ){  //dereferences symlink and checks if the entry exists
            if(errno == ENOENT)  
                syms_to_nowhere++;       
            else{
                print_err("***ERROR OPENING SYMLINK*** %s\n", pathname);
            }
        }
    }
 
    if(statbuf.st_nlink > 1 && (statbuf.st_mode & S_IFMT) != S_IFDIR){
        if(!check_inodes(statbuf.st_ino)){
            inodes++ && ino_g1++;  
            ino_type(statbuf);
            acc_reg_size(statbuf);
        }
    } else { // catch inodes w/ link == 1 or a dir
        inodes++;
        ino_type(statbuf);
        acc_reg_size(statbuf);
    }
    
    const char shell_prob_chars[] = "-~!;\"#$?&'*/<>\\`|%"; //- ~ are problematic if they are first char of a file name, else fine
    for(int i = 0; i < strlen(name); i++){ /* iterate through name of dirent and check if it contains problematic characters ... yes this is very computationally expensive*/
        if( !isalnum(name[i])){
            for(int j = (i == 0) ? 0 : 2; j < sizeof(shell_prob_chars); j++)  //ternary operator to only check - ~ as first char
                if(name[i] == shell_prob_chars[j] || !(isprint(name[i]) || isspace(name[i]) )){
                    problematic++;
                    return;
                }
        }
    }
}

int check_inodes(unsigned int check_inode)
{
    static unsigned int counted_inodes[INODE_BUFF] = {1,2}; 
    static int count = 2;

    if(count > (INODE_BUFF -1)){
        print_err("Toooooo many hardlinks\n", NULL);
        return 1;
    }

    for(int index = 0; index < count; index++)
        if(check_inode == counted_inodes[index]) 
            return 1; //inode was counted
            
    counted_inodes[count++] = check_inode;
    return 0; //inode had not been counted
}

void ino_type(struct stat statbuf){
    if(S_ISDIR(statbuf.st_mode))
        all_inodes[0]++;
    else if(S_ISCHR(statbuf.st_mode))
        all_inodes[1]++;
    else if(S_ISBLK(statbuf.st_mode))
        all_inodes[2]++;
    else if(S_ISREG(statbuf.st_mode))
        all_inodes[3]++;
    else if(S_ISFIFO(statbuf.st_mode))
        all_inodes[4]++;
    else if(S_ISLNK(statbuf.st_mode))
        all_inodes[5]++;
    else if(S_ISSOCK(statbuf.st_mode))
        all_inodes[6]++;
}

void acc_reg_size(struct stat statbuf){ // checks if inode is a regular file and counts the size and diskblocks used
    if((statbuf.st_mode & S_IFMT) == S_IFREG){
        tot_size += statbuf.st_size;
        disk_blocks += statbuf.st_blocks*(512.0/statbuf.st_blksize); // get the amount of disk_blocks being used by the file
    }
}

void print_err(const char *err_msg, const char *file){
    int errno_save = errno;
    fprintf(stderr, err_msg, file);
    if(errno_save != 0)
        fprintf(stderr, "\t(errno %d) %s\n", errno_save, strerror(errno_save));
}
