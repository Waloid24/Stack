#include <stdio.h>
#include <stdlib.h>
#include "my_assert.h"
#include <string.h>
 
typedef int elem_t;
typedef unsigned long long cnr_t;
typedef unsigned long long hash_t;

struct info_log {
    int    line;
    const char * name_func; 
    const char * name_file;
};

typedef struct {
    hash_t    stk_cnr_first; //stack_canary_first
    elem_t * data;
    size_t   n_memb;
    size_t   capacity;
    hash_t  * ptr_canary_data_first;
    cnr_t  * ptr_canary_data_second;
    cnr_t    stk_cnr_second;

    int      lineCreat;
    const char   * nameFuncCreat;
    const char   * nameFileCreat;
} stack_t;

static const elem_t poison = 0xBF;

const cnr_t BUF_CNR_SCND = 0xDEADBEEF;
const cnr_t STK_CNR_SCND = 0xBADF00D;

enum ERRORS {
    NOERR                =      0,
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
    reduce,      //0
    increase     //1
};

// #define poison_in_buf(num_in_poison, val_poison) \
//     *((elem_t*)((unsigned char *)stk->data + num_in_poison * sizeof(elem_t) + sizeof(cnr_t))) = val_poison

#define LONG_LINE "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n"

#define GET_INFO_STK()\
{\
    struct info_log info = \
    { \
    __LINE__,\
    __PRETTY_FUNCTION__, \
    __FILE__ \
    };\
    stk.lineCreat     = info.line;\
    stk.nameFuncCreat = info.name_func;\
    stk.nameFileCreat = info.name_file;\
}

//#define begin_buf (2*sizeof(cnr_t) + sizeof(buffer)) //in recalloc увеличиваем в 2 раза
#define RESIZE 2

#if (!defined(NDEBUG_STK) && defined(DEBUG_STK))
        #define LOGDUMP(canPrint, logFile, ptr_stk, text, isErr)\
                size_t num = dump_call_num ();\
                printf ("Number of the \"LOGDUMP\" function call is %zu\n", num);\
                if (isErr)\
                {\
                    fprintf (logFile, LONG_LINE);\
                    fprintf (logFile, "%s at %s(%d)\n", __PRETTY_FUNCTION__, __FILE__, __LINE__);\
                    fprintf (logFile, "Stack[%p](%d) \"" #ptr_stk "\" was created in file %s in function %s(str %d)\n", ptr_stk, isErr, (ptr_stk)->nameFileCreat, (ptr_stk)->nameFuncCreat, (ptr_stk)->lineCreat);\
                    fprintf (logFile, "%s\n", text);\
                    fprintf (logFile, LONG_LINE);\
                    printf ("Please, check log file \"log.txt\".\n");\
                }\
                else\
                    fprintf (logFile, "Everything is OK");\
                if (canPrint)\
                    dump (*(ptr_stk), logFile);\
                else \
                    printf ("Error information cannot be printed. Sorry.\n");\
                if (isErr)\
                    abort()
#endif

#if (defined(NDEBUG_STK) && !defined(DEBUG_STK))
        #define LOGDUMP(canPrint, logFile, ptr_stk, text, isErr)\
                size_t num = dump_call_num ();\
                printf ("Number of the \"LOGDUMP\" function call is %zu\n", num);\
                if (isErr)\
                {\
                    fprintf (logFile, LONG_LINE);\
                    fprintf (logFile, "%s at %s(%d)\n", __PRETTY_FUNCTION__, __FILE__, __LINE__);\
                    fprintf (logFile, "About \"" #ptr_stk "\": stack[%p](%d) has been creating at %s at %s(%d) \n", ptr_stk, isErr, (ptr_stk)->nameFileCreat, (ptr_stk)->nameFuncCreat, (ptr_stk)->lineCreat);\
                    fprintf (logFile, "%s\n", text);\
                    fprintf (logFile, LONG_LINE);\
                    printf ("Please, check log file \"log.txt\".\n");\
                }\
                else \
                    fprintf (logFile, "Everything is OK\n");\
                if (canPrint)\
                    dump (*(ptr_stk), logFile);\
                else \
                    printf ("Error information cannot be printed. Sorry.\n");
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
void   Ctor          (stack_t *  stk, size_t capacity);
void   Push          (stack_t *  stk, elem_t new_memb);
elem_t Pop           (stack_t *  stk);
void   SDtor         (stack_t *  stk);

//Output
void   dump          (stack_t stk, FILE * log_file);
size_t dump_call_num (void);

//Service function
void * recalloc      (void * memblock, size_t n_memb, size_t size_memb);
void   resize        (stack_t * stk, int mode);

//Calculate hashsum
void   hash_sum      (stack_t * stk);
hash_t calculateHash (void * object, size_t byteSize);

//Stack and buffer check
int    struct_validator (stack_t * stk);
void   decoder       (int value_elem);
void   log_ok        (void);

