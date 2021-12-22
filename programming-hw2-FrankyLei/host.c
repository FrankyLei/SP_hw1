#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main (int args , char* argv[]){
    if (args != 7){
        fprintf(stderr, "host args != 6, args = %d\n",args);
        _exit(0);
    }
    int host_id;
    int depth;
	int lucky_number;

    for (int i = 0 ; i < args ; i ++){
        if (strcmp(argv[i] ,"-m") == 0 ){
            host_id = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i] ,"-d") == 0 ){
            depth = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i] ,"-l") == 0 ){
            lucky_number = atoi(argv[i+1]);
        }
        
    }
//----------end taking input------------------------

    printf("[host] print form host_id = %d , d = %d , l = %d\n",host_id, depth , lucky_number);
    fflush(stdout); 
    return 0 ;
}