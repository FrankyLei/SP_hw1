#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc , char* argv[]){
    if (argc != 2){
        fprintf(stderr,"argc != 2\n");
        return 0 ; 
    }

    int player_id = atoi(argv[1]);  
    
    int guess;
    for(int round = 0 ; round < 10 ; round++){
        /* initialize random seed: */
        srand ((player_id + round) * 323);
        /* generate guess between 1 and 1000: */
        guess = rand() % 1001;

        printf("%d %d \n",player_id , guess);
        fflush(stdout);
    }
    return 0 ; 
}