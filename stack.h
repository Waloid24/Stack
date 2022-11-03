#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include "my_assert.h"
#include <string.h>
 
typedef int elem_t;
typedef unsigned long long cnr_t; // naming?
typedef unsigned long long hash_t;

struct info_log { // _t
    int    line;
    const char * name_func; 
    const char * name_file;
};

typedef struct {
    hash_t    stk_cnr_first; //stack_canary_first // useless commentary
    elem_t * data;
    size_t   n_memb;
    size_t   capacity;
    const char * name;
    size_t min_capacity; // snake or camel-case?
    hash_t  * ptr_canary_data_first;
    cnr_t  * ptr_canary_data_second;
    int      lineCreat;
    const char   * nameFuncCreat;
    const char   * nameFileCreat;
    cnr_t    stk_cnr_second;

} stack_t;
//ToDo: хранить хэш буфера в структуре (?)

static const elem_t POISON = 0xBF;

const cnr_t BUF_CNR_SCND = 0xDEADBEEF; // ToDo: needs 8 bytes, not 4
const cnr_t STK_CNR_SCND = 0xBADF00D;

enum ERRORS {
    NOERR                =      0, // 
    // ToDo: where is 1 << 0?
    // ToDo: GLOBAL_CONST_CAPS
    ptr_stk_null         = 1 << 1, //
    ptr_buf_null         = 1 << 2, //
    ptr_log_null         = 1 << 3, 
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

enum isErr {
    NO = 0,
    YES = 1
};

enum MODE {
    reduce = 0,
    increase = 1
    };

enum is_abort {
    no_abort = 0,
    yes_abort = 1
};

#define LONG_LINE "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n"

#define GET_INFO_STK()\
{\
    struct info_log info = \
    { \
    __LINE__,\
    __PRETTY_FUNCTION__, \
    __FILE__ \
    };\
    stk.lineCreat     = info.line; /*ToDo: it isn't true!!!!*/\
    stk.nameFuncCreat = info.name_func;\
    stk.nameFileCreat = info.name_file;\
}

const int RESIZE = 2; 
const int THRESHOLD_RATIO = 4;

// FIXME:

#if (!defined(NDEBUG_STK) && defined(DEBUG_STK))
    #define LOGDUMP(canPrint, logFile, ptr_stk, message, is_err)\
        logdump_hidden(canPrint, logFile, ptr_stk, message, is_err, yes_abort, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

#if (defined(NDEBUG_STK) && !defined(DEBUG_STK))
    #define LOGDUMP(canPrint, logFile, ptr_stk, message, is_err)\
        logdump_hidden(canPrint, logFile, ptr_stk, message, is_err, no_abort, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif 

#if (!defined(NDEBUG_STK) && !defined(DEBUG_STK))
        #define LOGDUMP(canPrint, logFile, ptr_stk, text, isErr)\
        {\
            printf ("Please select one of the two modes (NDEBUG_STK or DEBUG_STK) and restart the program.\n"); \
            abort();\
        }
#endif

//STK_OK -> struct_validator -> decoder
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
void   stack_ctor          (stack_t *  stk, size_t capacity, const char * name_stk);
void   stack_push          (stack_t *  stk, elem_t new_memb);
elem_t stack_pop           (stack_t *  stk);
void   stack_Dtor         (stack_t *  stk);

//Output
void   stack_dump        (stack_t stk, FILE * log_file);
size_t dump_call_num (void);

//Service function
void * stack_recalloc      (void * memblock, size_t n_memb, size_t size_memb);
void   stack_resize        (stack_t * stk, int mode);
void   print_code_err(int * error_number);

void logdump_hidden (unsigned char can_print, FILE * stack_log, stack_t * stk, const char * message, 
                     unsigned char is_err, unsigned char is_abort,  const char * call_func, 
                     const char* call_file, unsigned int call_line);

//Calculate hashsum
void   stack_hash_sum      (stack_t * stk);
hash_t calculate_hash (void * object, size_t byte_size);

//Stack and buffer check
int    struct_validator (stack_t * stk);
void   decoder       (int value_elem);
void   log_ok        (void);

#endif