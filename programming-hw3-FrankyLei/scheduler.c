#include "threadtools.h"

/*
1) You should state the signal you received by: printf('TSTP signal caught!\n') or printf('ALRM signal caught!\n')
2) If you receive SIGALRM, you should reset alarm() by timeslice argument passed in ./main
3) You should longjmp(SCHEDULER,1) once you're done.
*/
void sighandler(int signo){
	/* Please fill this code section. */
	if (signo == SIGTSTP){
		printf("TSTP signal caught!\n");
	}
	else{
		printf("ALRM signal caught!\n");
		alarm(timeslice);//reset timer
	}
	longjmp(SCHEDULER , 1);
}

/*
1) You are stronly adviced to make 
	setjmp(SCHEDULER) = 1 for ThreadYield() case
	setjmp(SCHEDULER) = 2 for ThreadExit() case
2) Please point the Current TCB_ptr to correct TCB_NODE
3) Please maintain the circular linked-list here
*/
void del_cur_liked_list(){
	// only one thread
	if ( Current == Current -> Prev && Current == Current -> Next){
		if (Current == Head){
			Head = NULL;
		}
		free(Current);
	}
	else{//still other thread left, maintain the circular linked-list 
		TCB_ptr tem_ptr = Current;
		Current -> Prev -> Next = Current -> Next ;
		Current -> Next -> Prev = Current -> Prev;
		if (Current == Head ){
			Head = Head -> Next;
		}
		Current = Current -> Next;
		free(tem_ptr);
	}
}

void scheduler(){
	Current = Head ;
	
	while(1){
		int val = setjmp(SCHEDULER);
		if (val == 0 ){
			longjmp(Current -> Environment , 1 );
		}
		else if(val == 1){//	thread yield
			Current = Current -> Next;//	do the next thread 
		}
		else{//	thread exit 
			del_cur_liked_list();
		}

		if (Head == NULL){//	no thread left 
			break; 
		}
	}
	longjmp(MAIN , 1);//finished,	return to main and end
}
