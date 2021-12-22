#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

/*
ref:http://www.csl.mtu.edu/cs4411.ck/common/Coroutines.pdf
*/

extern int timeslice, switchmode;

typedef struct TCB_NODE *TCB_ptr;
typedef struct TCB_NODE{
    jmp_buf  Environment;
    int      Thread_id;
    TCB_ptr  Next;
    TCB_ptr  Prev;
    int i, N;
    int w, x, y, z;
} TCB;

extern jmp_buf MAIN, SCHEDULER;
extern TCB_ptr Head;
extern TCB_ptr Current;
extern TCB_ptr Work;
extern sigset_t base_mask, waiting_mask, tstp_mask, alrm_mask;

void sighandler(int signo);
void scheduler();

#define insert_cur_linked_list(){\
    if (Head == NULL){\
        Work -> Prev = Work;\
        Work -> Next = Work;\
        Head = Work;\
    }\
    else{\
        Work -> Prev = Head -> Prev;\
        Work -> Next = Head;\
        Head -> Prev -> Next = Work ;\
        Head -> Prev = Work;\
    }\
}\

// Call function in the argument that is passed in
#define ThreadCreate(function, thread_id, number){\
	int val = setjmp(MAIN);\
    if (val == 0 ){\
        function(thread_id, number);\
    }\
}\

// Build up TCB_NODE for each function, insert it into circular linked-list
#define ThreadInit(thread_id, number)\
{                                                                                         \
	/* Please fill this code section. */\
    Work = malloc(sizeof(struct TCB_NODE));\
    Work -> Thread_id = thread_id;\
    /*init*/ \
    Work -> N = number ;Work -> z = 0;\
    /*init*/ \
    insert_cur_linked_list();\
    int val = setjmp(Work -> Environment);\
    if (val == 0){\
        longjmp(MAIN,1);\
    }\
}\


// Call this while a thread is terminated
#define ThreadExit()                                                                      \
{                                                                                         \
	longjmp(SCHEDULER , 2);												                  \
}                                                                                           \

// Decided whether to "context switch" based on the switchmode argument passed in main.c

#define context_by_input(){\
    int val = setjmp(Current -> Environment);\
    if(val == 0){\
        sigset_t sigset;\
        sigemptyset(&sigset);\
        sigpending(&sigset);\
        if(sigismember(&sigset , SIGTSTP)){\
            sigsuspend(&alrm_mask);\
        }\
        else if (sigismember(&sigset , SIGALRM)){\
            sigsuspend(&tstp_mask);\
        }\
    }\
}\

#define ThreadYield()                                                                     \
{                                                                                         \
	if (switchmode == 0 ){												  \
        int val = setjmp(Current -> Environment);\
        if(val == 0){\
            longjmp(SCHEDULER , 1);\
        }\
    }\
    else{\
        context_by_input();\
    }\
}\

