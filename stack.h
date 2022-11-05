#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include "my_assert.h"
#include <string.h>
 
typedef int elem_t;
typedef unsigned long long canary_t;
typedef unsigned long long hash_t;

struct info_log_t { 
    int          line;
    const char * name_func; 
    const char * name_file;
};

typedef struct {
    hash_t       hashsum_stack;
    elem_t     * data;
    size_t       n_memb;
    size_t       capacity;
    const char * name;
    size_t       min_capacity; 
    hash_t     * ptr_canary_hashsum;
    canary_t      * ptr_canary_data;
    size_t       line_create;
    const char * name_func_create;
    const char * name_file_create;
    canary_t        stk_cnr_second;

} stack_t;

const elem_t POISON_ELEM_STK = 0xBF;
const canary_t BUF_CNR_SCND = 0xDEADBEEF;
const canary_t STK_CNR_SCND = 0xBADF00D;
const int RESIZE = 2; 
const int THRESHOLD_RATIO = 4;

enum STK_ERRORS {
    NOERR_STK                =      0, //
    PTR_STK_NULL         = 1 << 0, //
    PTR_BUF_NULL_STK         = 1 << 1, //
    PTR_LOG_NULL_STK         = 1 << 2, 
    SIZE_MORE_CAPACITY_STK   = 1 << 3, //
    BAD_BUF_CAN_SCND_STK     = 1 << 4, //
    BAD_STK_CAN_SCND     = 1 << 5, //
    BAD_BUF_HASH_STK         = 1 << 6, //
    BAD_STK_HASH         = 1 << 7, //
    BAD_PTR_BUF_HASH_STK     = 1 << 8, //
    BAD_PTR_BUF_CANARY_STK   = 1 << 9, //
    BAD_POISON_STK           = 1 << 10 //
};

enum is_error {
    NO = 0,
    YES = 1
};

enum MODE {
    REDUCE = 0,
    INCREASE = 1
    };

enum is_abort {
    NO_ABORT = 0,
    YES_ABORT = 1
};

//-------------------------------------------------DEFINES----------------------------------------------------- 

#define stk_ctor(stk, capacity, name_stk)\
        stack_ctor(stk, capacity, name_stk, __LINE__, __PRETTY_FUNCTION__, __FILE__)

#if (!defined(NDEBUG_STK) && defined(DEBUG_STK))
    #define LOGDUMP(canPrint, logFile, ptr_stk, message, is_err)\
        logdump_hidden(canPrint, logFile, ptr_stk, message, is_err, YES_ABORT, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

#if (defined(NDEBUG_STK) && !defined(DEBUG_STK))
    #define LOGDUMP(canPrint, logFile, ptr_stk, message, is_err)\
        logdump_hidden(canPrint, logFile, ptr_stk, message, is_err, NO_ABORT, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif 

#if (!defined(NDEBUG_STK) && !defined(DEBUG_STK))
        #define LOGDUMP(canPrint, logFile, ptr_stk, text, is_err)\
        {\
            printf ("Please select one of the two modes (NDEBUG_STK or DEBUG_STK) and restart the program.\n"); \
            abort();\
        }
#endif

//----------------------------------------------FUNCTIONS----------------------------------------------------
//Creating and changing the stack
void   stack_ctor       (stack_t * stk, size_t capacity, const char * name_stk, 
                         size_t create_line, const char * create_func, const char * create_file);
void   stack_push       (stack_t * stk, elem_t new_memb, FILE * log);
elem_t stack_pop        (stack_t * stk, FILE * log);
void   stack_Dtor       (stack_t *  stk);

//Output
void   stack_dump       (stack_t stk, FILE * log_file);
size_t dump_call_num    (void);

//Service function
void * stack_recalloc   (void * memblock, size_t n_memb, size_t size_memb);
void   stack_resize     (stack_t * stk, int mode);
void   print_code_err   (int * error_number);

void   logdump_hidden   (unsigned char can_print, FILE * stack_log, stack_t * stk, const char * message, 
                         unsigned char is_err, unsigned char is_abort,  const char * call_func, 
                         const char* call_file, unsigned int call_line);

FILE * open_logfile     (const char * name_logfile);

//Calculate hashsum
void   stack_hash_sum   (stack_t * stk);
hash_t calculate_hash   (void * object, size_t byte_size);

//Stack and buffer check
int    struct_validator (stack_t * stk, FILE * log);
void   decoder          (int value_elem);
void   log_ok           (void);
void   stk_ok           (stack_t * stk, FILE * log);


#endif