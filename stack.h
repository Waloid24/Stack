#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include "my_assert.h"
#include <string.h>
 
typedef int elem_t;
typedef unsigned long long cnr_t; // naming?
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
    cnr_t      * ptr_canary_data;
    size_t       line_create;
    const char * name_func_create;
    const char * name_file_create;
    cnr_t        stk_cnr_second;

} stack_t;

static const elem_t POISON = 0xBF;

const cnr_t BUF_CNR_SCND = 0xDEADBEEF;
const cnr_t STK_CNR_SCND = 0xBADF00D;

enum ERRORS {
    NOERR                =      0, // 
    // ToDo: where is 1 << 0?
    // ToDo: GLOBAL_CONST_CAPS
    PTR_STK_NULL         = 1 << 1, //
    PTR_BUF_NULL         = 1 << 2, //
    PTR_LOG_NULL         = 1 << 3, 
    bad_capacity         = 1 << 4, //
    bad_size             = 1 << 5, //
    size_more_capac      = 1 << 6, //
    bad_buf_can_scnd     = 1 << 7, //
    bad_stk_can_scnd     = 1 << 8, //
    bad_buf_hash         = 1 << 9, //
    bad_stk_hash         = 1 << 10,//
    bad_ptr_buf_hash     = 1 << 11,//
    bad_ptr_buf_can_scnd = 1 << 12,//
    bad_poison           = 1 << 13 //
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

#define LONG_LINE "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n"

#define STK_CTOR(stk, capacity, name_stk)\
        stack_ctor(stk, capacity, name_stk, __LINE__, __PRETTY_FUNCTION__, __FILE__)

const int RESIZE = 2; 
const int THRESHOLD_RATIO = 4;

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

#define STK_OK(ptr_stk) \
        do { \
            printf ("STK_OK\n"); \
            int sum_err = 0; \
            if ((sum_err = struct_validator(ptr_stk)) != NOERR) \
            { \
                printf ("Check log file \"log.txt\", you have some problems (with your head)\n"); \
                decoder (sum_err); \
            } \
        } while (0);

////---------------------------------------------------------------------------------------------
//Creating and changing the stack
void   stack_ctor       (stack_t * stk, size_t capacity, const char * name_stk, 
                         size_t create_line, const char * create_func, const char * create_file);
void   stack_push       (stack_t *  stk, elem_t new_memb);
elem_t stack_pop        (stack_t *  stk);
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

//Calculate hashsum
void   stack_hash_sum   (stack_t * stk);
hash_t calculate_hash   (void * object, size_t byte_size);

//Stack and buffer check
int    struct_validator (stack_t * stk);
void   decoder          (int value_elem);
void   log_ok           (void);

#endif