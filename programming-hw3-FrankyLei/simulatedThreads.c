#include "threadtools.h"

// Please complete this three functions. You may refer to the macro function defined in "threadtools.h"

// Mountain Climbing
// You are required to solve this function via iterative method, instead of recursion.

/*
MountainClimbing(): Imagine you are climbing a mountain. Each step you can either climb 1 or 2 units of height. 
Please compute how many distinct ways you climb to the top? The function stops when the calculation is done.
*/

// Current -> N save number****

void MountainClimbing(int thread_id, int number){
	/* Please fill this code section. */
	
	//init
	//printf("MC number 1------before init----- %d\n", number);
	ThreadInit(thread_id , number);
	//printf("MC number 2----------- %d\n", number);
	Current -> x = 1 ;
	Current -> y = 1 ;
	//Current -> N = number ;
	//printf("number = %d\n", Current -> N);

	for (Current->i= 0 ; Current->i < (Current->N -1 ) ; Current->i++){//dp  2 3 5 8 13 ...
		sleep(1);
		Current -> z = Current -> x + Current -> y ;
		printf("Mountain Climbing: %d\n",Current->z);

		Current -> x = Current -> y ;
		Current -> y = Current -> z ; 
		
		ThreadYield();
	}

	ThreadExit();

}

// Reduce Integer
// You are required to solve this function via iterative method, instead of recursion.
void ReduceInteger(int thread_id, int number){
	/* Please fill this code section. */
	// printf("RE number 1-----before init------ %d\n", number);
	ThreadInit(thread_id , number);
	// printf("RE number 2----------- %d\n", number);
	//Current -> x = number;
	//current -> y be counter 
	//printf("number x = %d\n", Current -> x);

	while(1){
		sleep(1);
		// printf("rd sth \n");
		if (Current -> N == 1 ){//finish 
			break;
		}

		if (Current -> N % 2 == 0 ){//even
			Current -> N /= 2;
			Current -> y ++;
		}
		else{
			Current -> N -- ;
			Current -> y ++;
		}
		printf("Reduce Integer: %d\n", Current -> y );
		//printf("red\n");
		ThreadYield();
	}
	ThreadExit();
}


// Operation Count
// You are required to solve this function via iterative method, instead of recursion.
void OperationCount(int thread_id, int number){
	// printf("OP number 1-----before init------ %d\n", number);
	ThreadInit(thread_id , number );
	// printf("OP number 2----------- %d\n", number);
	//Current -> x = number ;
	Current -> y = Current -> N  + 322; //avg
	//Current -> N = number;//len
	//printf("number NN = %d\n", Current -> N);
	//Current -> z = 0; //counter 
	
		
	for ( Current -> i = 0; Current -> i < Current -> N; Current -> i++){
		sleep(1);

		Current -> w = 2 * Current -> i + 323;//val in index
		while( (Current -> w < Current -> y) && (Current -> i < Current -> N)){
			Current -> i++;
			// printf("iter %d\n",Current -> i );
			Current -> w = 2 * Current -> i + 323;//val in index
		}

		Current -> z += (Current -> w - Current -> y );
		printf("Operation Count: %d\n", Current -> z);	
		ThreadYield();
		// printf("OperationCount :%d\n", Current -> z);//test 
		// ThreadYield();
	}
	ThreadExit();
}
