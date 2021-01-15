#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv)
{
    int gen_len = -1; /* endless loop */

    if (argc > 2){
        fprintf(stderr, "%s usage: how many words to generate or no arguments\n", argv[0]);
        exit(-1);
    } else if (argc == 2){
        gen_len = atoi(argv[1]);
    }

    char word[16];
    int word_size;
    srand(time(NULL));

    for(int i = 0;; i++) /* potential infinite loop */
    { 
        if(gen_len != -1 && i >= gen_len) break; 

        word_size = (rand() % (13)) + 2;
        int j;
        for(j = 0; j < word_size; j++){
            word[j] = (char) rand() % 26 + 'A'; /* all capital letters  */
        }
        word[++j] = '\0'; 

        printf("%s\n", word);
    }

    return 0;
}
