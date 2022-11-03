
#include <stdio.h>

#include "stack.h"

int main (void)
{
    //FILE* logstream = openLog("log.txt");

    // stack_dump(logstream, &stk);

    // struct_validator (nullptr);

    stack_t stk = {};
    log_ok();
    FILE* logfile = fopen ("log.txt", "a");

    //printf ("%p\n", logfile);
    GET_INFO_STK();
    //printf ("After GET\n");
    //printf ("After first dump\n");

    stack_ctor (&stk, 2, "stk");
    stack_dump(stk, logfile);
    //printf ("in main\n");

    //printf ("0. *(stk.ptr_canary_data_first) = %llX\n", *(stk.ptr_canary_data_first));
    stack_push (&stk, 4);
    //printf ("1. *(stk.ptr_canary_data_first) = %llX\n", *(stk.ptr_canary_data_first));
    stack_push (&stk, 5);
    //printf ("2. *(stk.ptr_canary_data_first) = %llX\n", *(stk.ptr_canary_data_first));
    stack_push (&stk, 6);
    //printf ("3. *(stk.ptr_canary_data_first) = %llX\n", *(stk.ptr_canary_data_first));

    stack_dump(stk, logfile);
    //printf ("in main\n");

    elem_t c = stack_pop (&stk);
    //printf ("c = %d\n", c);
    
    stack_Dtor (&stk);
    //dump(stk, logfile);

    return 0;
}
