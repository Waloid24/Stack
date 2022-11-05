
#include <stdio.h>

#include "stack.h"

int main (void)
{
    //FILE* logstream = openLog("log.txt");

    // stack_dump(logstream, &stk);

    // struct_validator (nullptr);

    stack_t stk = {};
    log_ok();
    FILE * logfile = open_logfile ("log.txt");

    //printf ("%p\n", logfile);
    //printf ("After GET\n");
    //printf ("After first dump\n");
    fprintf (logfile, "Hi!\n");
    STK_CTOR(&stk, 2, "stk");
    stack_dump(stk, logfile);
    //printf ("in main\n");

    printf ("0. *(stk.ptr_canary_hashsum) = %llX\n", *(stk.ptr_canary_hashsum));
    stack_push (&stk, 4, logfile);
    printf ("1. *(stk.ptr_canary_hashsum) = %llX\n", *(stk.ptr_canary_hashsum));
    stack_dump(stk, logfile);
    //stack_push (&stk, 5, logfile);
    //printf ("2. *(stk.ptr_canary_hashsum) = %llX\n", *(stk.ptr_canary_hashsum));
    //stack_push (&stk, 6, logfile);
    //printf ("3. *(stk.ptr_canary_hashsum) = %llX\n", *(stk.ptr_canary_hashsum));

    // stack_dump(stk, logfile);
    // //printf ("in main\n");

    // elem_t c = stack_pop (&stk, logfile);
    // //printf ("c = %d\n", c);
    
    // stack_Dtor (&stk);
    //dump(stk, logfile);

    fclose(logfile);

    return 0;
}


