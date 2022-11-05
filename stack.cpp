#define DEBUG_STK
#define LONG_LINE "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n"
#include "stack.h"


void stack_ctor (stack_t * stk, size_t capacity, const char * name_stk, 
                 size_t create_line, const char * create_func, const char * create_file)
{
    stk -> n_memb = 0;
    stk -> capacity = capacity;
    stk -> min_capacity = capacity;
    stk -> name = name_stk;

    stk->line_create      = create_line;
    stk->name_func_create = create_func;
    stk->name_file_create = create_file;

    stk -> stk_cnr_second = STK_CNR_SCND;

    char * data = (char *) calloc (2*sizeof(canary_t) + (stk->capacity)*sizeof(elem_t), sizeof(char));

    stk -> ptr_canary_hashsum  = (canary_t *) data;
    stk -> data = (elem_t *) ((char *)data + sizeof(canary_t));
    stk -> ptr_canary_data = (canary_t *) ((char *)data + sizeof(canary_t) + (stk->capacity)*sizeof(elem_t));
    *(stk -> ptr_canary_data) = BUF_CNR_SCND;

    //printf ("In Ctor: ");
    for (size_t i = 0; i < stk->capacity; i++)
    {
        *((elem_t*)((char *)stk->data + i * sizeof(elem_t))) = POISON_ELEM_STK;
        //printf ("%X, ", *((elem_t*)((char *)stk->data + i * sizeof(elem_t))));
    }
    //printf ("\n");

    *(stk -> ptr_canary_hashsum) = calculate_hash (stk->data, sizeof(sizeof(hash_t) + (stk->capacity)*sizeof(elem_t)));
    stk -> hashsum_stack = calculate_hash ((char*)stk + (char) sizeof (canary_t), sizeof(stack_t)-sizeof(canary_t));
    
} 

//^^^^^^^^^^^^^^^^^^^^
void stack_push (stack_t * stk, elem_t new_memb, FILE * log)
{
    MY_ASSERT (stk == nullptr, "No stack access");
    MY_ASSERT (log == nullptr, "There is no access to logfile");
    
    if (stk->n_memb >= stk->capacity)
    {
        stack_resize (stk, INCREASE);
    }
    stk->data[stk->n_memb] = new_memb;
    stk->n_memb++;

    stack_hash_sum (stk);
    stk_ok (stk, log);
    // fclose (logfile);
}

elem_t stack_pop (stack_t * stk, FILE * log)
{
    stk->n_memb--;

    stack_hash_sum(stk);
    MY_ASSERT (stk->n_memb < 0, "Error : there are no variables in the stack"); 

    elem_t pop = 0;
    if (stk->n_memb >= 0)
    {
        pop = (stk->data)[stk->n_memb];
        (stk->data)[stk->n_memb] = POISON_ELEM_STK;
    }
    
    if (THRESHOLD_RATIO*(stk->n_memb) <= stk->capacity && (stk->capacity/RESIZE) >= (stk -> min_capacity)) 
    {
        stack_resize(stk, REDUCE);
    }

    stack_hash_sum(stk);
    stk_ok(stk, log);

    return pop;
}

void stack_Dtor (stack_t * stk)
{
    stk->capacity       = 0;
    stk->n_memb         = 0;
    stk->stk_cnr_second = 0;
    stk->hashsum_stack  = 0;
    stk->min_capacity   = 0;
    stk->stk_cnr_second = 0;

    free(stk->ptr_canary_hashsum);

    stk->ptr_canary_hashsum = nullptr;
    stk->ptr_canary_data    = nullptr;
    stk->name_file_create      = nullptr;
    stk->name_func_create      = nullptr;
    stk->line_create        = 0;

    stk = nullptr;
}

void stack_dump (stack_t stk, FILE * log_file) 
{
    if (log_file == nullptr)
    {
        log_file = stdout;
    }

    if (stk.capacity == 0)
    {
        printf ("stk.capacity = 0, maybe you have used the function Dtor\n");
        abort();
    }

    //printf ("1\n");
    fprintf (log_file, LONG_LINE);
    fprintf (log_file, "Hashsum of stack  = %llx\n", stk.hashsum_stack);
    //printf ("2\n");
    fprintf (log_file, "stk_cnr_second = %llx\n", stk.stk_cnr_second);
    //printf ("3\n");
    fprintf (log_file, "\nArray in struct:\n");
    //printf ("4\n");

    fprintf (log_file, "Hashsum of data: %llX\n", *(stk.ptr_canary_hashsum));
    fprintf (log_file, "In data:");
    for (size_t i = 0; i < stk.capacity; i++)
    {
        fprintf (log_file, "%X, ", *((elem_t*)((char *)stk.data + i * sizeof(elem_t))));
    }
    fprintf (log_file, "\n");
    fprintf (log_file, "Second canary in data: %llX\n", *(stk.ptr_canary_data));

    
    //printf ("5\n");
    fprintf (log_file, "\nNumber of members = %zu, capacity = %zu\n", stk.n_memb, stk.capacity);
    fprintf (log_file, "stk was created in file = %s, in func = %s, in strings = %zu\n", stk.name_file_create, stk.name_func_create, stk.line_create);
    fprintf (log_file, LONG_LINE);
    fflush  (log_file);
}

size_t dump_call_num (void)
{
    static size_t count = 0;
    count++;
    return count;
}

void * stack_recalloc (void * memblock, size_t n_memb, size_t size_memb)
{
    if (n_memb < 0)
    {
        free (memblock);
        return memblock;
    }
    if (memblock == nullptr)
    {
        return calloc (n_memb, size_memb);        
    }
    else    
    {
        return realloc (memblock, n_memb*size_memb);
    }
}

void stack_resize (stack_t * stk, int mode)
{
    if (mode == INCREASE)
    {
        stk->capacity *= RESIZE;
        canary_t temp     = *(stk->ptr_canary_data);
        //printf ("in resize before recalloc stk->ptr_canary_data = %p\n", stk->ptr_canary_data);
        //printf ("The value of stk->ptr_canary_data BEFORE resize = %llX\n", *(stk->ptr_canary_data));

        char * ptr = (char *) stack_recalloc (stk->ptr_canary_hashsum, (stk->capacity)*sizeof(elem_t) + 2*sizeof(hash_t), sizeof(char));

        stk->ptr_canary_hashsum     = (hash_t *) ptr;
        stk->data                      = (elem_t *)((char *)stk->ptr_canary_hashsum + sizeof (hash_t));
        stk->ptr_canary_data    = (canary_t *)((char *)stk->ptr_canary_hashsum + (char)((stk->capacity)*sizeof(elem_t)) + sizeof(hash_t));
        *(stk->ptr_canary_data) = temp;

        MY_ASSERT (stk->ptr_canary_hashsum == nullptr, "New pointer after resize is nullptr");

        for (int i = 0; i < (stk->capacity - stk->n_memb); i++)
        {
            *((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb) = POISON_ELEM_STK;
        }

        //stk->ptr_canary_hashsum = (canary_t *) ptr;
        //printf ("in resize after recalloc stk->ptr_canary_data = %p\n", stk->ptr_canary_data);
        //printf ("The value of stk->ptr_canary_data AFTER resize = %llX\n", *(stk->ptr_canary_data));
        //check_err (); stk->data != nullptr
    }
    else if (mode == REDUCE)
    {
        stk->capacity /= RESIZE;
        canary_t temp = *(stk->ptr_canary_data);

        //printf ("in resize before recalloc stk->ptr_canary_data = %p\n", stk->ptr_canary_data);
        //printf ("The value of stk->ptr_canary_data BEFORE resize = %llX\n", *(stk->ptr_canary_data));

        char * ptr = (char *) stack_recalloc (stk->ptr_canary_hashsum, (stk->capacity)*sizeof(elem_t) + 2*sizeof(hash_t), sizeof(char));

        stk->ptr_canary_hashsum     = (hash_t *) ptr;
        stk->data                      = (elem_t *)((char *)stk->ptr_canary_hashsum + sizeof (hash_t));
        stk->ptr_canary_data    = (canary_t  *)((char *)stk->ptr_canary_hashsum + (char)((stk->capacity)*sizeof(elem_t)) + sizeof(hash_t));
        *(stk->ptr_canary_data) = temp;

        MY_ASSERT (stk->ptr_canary_hashsum == nullptr, "New pointer after resize is nullptr");

        for (int i = 0; i < (stk->capacity - stk->n_memb); i++)
        {
            *((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb) = POISON_ELEM_STK;
        }

        //stk->ptr_canary_hashsum = (canary_t *) ptr;
        //printf ("in resize after recalloc stk->ptr_canary_data = %p\n", stk->ptr_canary_data);
        //printf ("The value of stk->ptr_canary_data AFTER resize = %llX\n", *(stk->ptr_canary_data));
        //check_err (); stk->data != nullptr
    }
    else 
    {
        printf ("You try to break my stack! .!. you\n");
        abort ();
    }
    
    stack_hash_sum (stk);
}

void stack_hash_sum (stack_t * stk)
{
    MY_ASSERT (stk == nullptr, "There is no access to stack");

    *(stk -> ptr_canary_hashsum) = calculate_hash (stk->data, sizeof(sizeof(hash_t) + (stk->capacity)*sizeof(elem_t)));
    //printf ("Hashsum of data = %llX\n", *(stk -> ptr_canary_hashsum));
    stk -> hashsum_stack = calculate_hash ((char*)stk + (char) sizeof (canary_t), sizeof(stack_t)-sizeof(canary_t));
    //printf ("Hashsum of stk = %llX\n", stk -> hashsum_stack);
}

hash_t calculate_hash (void * object, size_t byte_size)
{
    MY_ASSERT (object == nullptr, "There is no access to the object from which the hash sum needs to be calculated");

    char * start_ptr = (char *) object;
    hash_t hash_sum = 0xDED60D;

    for (char i = 0; i < byte_size; i++)
    {
        hash_sum += i * start_ptr[i];
    }

    return hash_sum;
}
// ^^^^^^^^^^^^^^^
int struct_validator (stack_t * stk, FILE * log)
{
    log_ok ();

    int err_sum = NOERR_STK;
    //printf ("inside str_validator\n");
    if (stk == nullptr)
    {
        //printf ("stk == nullptr\n");
        LOGDUMP(NO, log, stk, "The pointer to the stack is null", YES);
        fclose (log);
        err_sum |= PTR_STK_NULL;
    }
    if (stk->n_memb > stk->capacity)
    {
        //printf ("stk->n_memb > stk->capacity\n");
        LOGDUMP(YES, log, stk, "The number of members in the buffer is greater than its capacity", YES);
        fclose (log);
        free (stk->data);
        err_sum |= SIZE_MORE_CAPACITY_STK;
    }
    if (stk->data == nullptr)
    {
        //printf ("stk->data == nullptr\n");
        LOGDUMP(NO, log, stk, "The pointer to buffer is null", YES);
        fclose (log);
        err_sum |= PTR_BUF_NULL_STK;
    } 
    if (stk->ptr_canary_hashsum == nullptr)
    {
        //printf ("stk->ptr_canary_hashsum == nullptr\n");
        LOGDUMP(NO, log, stk, "The pointer to hash-sum of buffer is null", YES);
        fclose (log);
        err_sum |= BAD_PTR_BUF_HASH_STK;
    }
    if (stk->ptr_canary_data == nullptr)
    {
        //printf ("stk->ptr_canary_data == nullptr\n");
        LOGDUMP(NO, log, stk, "The pointer to second buffer canary is null", YES);
        fclose (log);
        err_sum |= BAD_PTR_BUF_CANARY_STK;
    }
    if (*(stk->ptr_canary_hashsum) != calculate_hash (stk->data, sizeof(sizeof(hash_t) + (stk->capacity)*sizeof(elem_t))))
    {
        //printf ("*(stk->ptr_canary_hashsum) = %llX\n", *(stk->ptr_canary_hashsum));
        //printf ("stk->hash_buf != calculate_hash\n");
        LOGDUMP(YES, log, stk, "Hashsum of buffer isn't correct", YES);
        fclose (log);
        err_sum |= BAD_BUF_HASH_STK;
    }
    if (*(stk->ptr_canary_data) != BUF_CNR_SCND)
    {
        //printf ("*(stk->ptr_canary_data) != BUF_CNR_SCND\n");
        //printf ("*(stk->ptr_canary_data) = %llX\n", *(stk->ptr_canary_data));
        LOGDUMP(YES, log, stk, "The second buffer canary died", YES);
        fclose (log);
        err_sum |= BAD_BUF_CAN_SCND_STK;
    }
    if (stk->hashsum_stack != calculate_hash ((char*)stk + (char) sizeof (canary_t), sizeof(stack_t)-sizeof(canary_t)))
    {
        //printf ("stk->hashsum_stack == %llX", calculate_hash ((char*)stk + (char) sizeof (canary_t), sizeof(stack_t)-sizeof(canary_t)));
        //printf ("stk->hashsum_stack != calculate_hash\n");
        LOGDUMP(YES, log, stk, "Hashsum of stack isn't correct", YES);
        fclose (log);
        err_sum |= BAD_STK_HASH;
    }
    if (stk->stk_cnr_second != STK_CNR_SCND)
    {
        //printf ("stk->stk_cnr_second != STK_CNR_SCND\n");
        LOGDUMP(YES, log, stk, "The second canary died", YES);
        fclose (log);
        err_sum |= BAD_STK_CAN_SCND;
    }
    for (int i = 0; i < (stk->capacity - stk->n_memb); i++)
    {
        if (*((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb) != POISON_ELEM_STK)
        {
            //printf ("*((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb) = %d\n", *((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb));
            //printf ("Poison isn't poison! ^_^\n");
            LOGDUMP(YES, log, stk, "The value of the poison cell has been changed", YES);
            fclose (log);
            err_sum |= BAD_POISON_STK;
        }
    }
    // LOGDUMP (YES, log, stk, "Hashsum of stack isn't correct", NO);
    
    return err_sum;
}
// использовать только перед struct_validator, ведь только там он используется, потом undef
void decoder (int value_elem)
{
    int count_of_err[32] = {0};

    if ((value_elem & PTR_STK_NULL)         == 1) 
    {
        count_of_err[1] = 1;
    }
    if ((value_elem & PTR_BUF_NULL_STK)         == 2)
    {
        count_of_err[2] = 1;
    }
    if ((value_elem & PTR_LOG_NULL_STK)         == 4)
    {
        count_of_err[3] = 1;
    }
    if ((value_elem & SIZE_MORE_CAPACITY_STK)   == 8)
    {
        count_of_err[4] = 1;
    }
    if ((value_elem & BAD_BUF_CAN_SCND_STK)     == 16)
    {  
        count_of_err[5] = 1;
    }
    if ((value_elem & BAD_STK_CAN_SCND)     == 32)
    {
        count_of_err[6] = 1;
    }
    
    if ((value_elem & BAD_BUF_HASH_STK)         == 64)
    {
        count_of_err[7] = 1;
    }
    if ((value_elem & BAD_STK_HASH)         == 128)
    {
        count_of_err[8] = 1;
    }
    if ((value_elem & BAD_PTR_BUF_HASH_STK)     == 256)
    {
        count_of_err[9] = 1;
    }
    if ((value_elem & BAD_PTR_BUF_CANARY_STK)   == 512)
    {
        count_of_err[10] = 1;
    }
    if ((value_elem & BAD_POISON_STK)           == 1024)
    {
        count_of_err[11] = 1;
    }

    print_code_err (count_of_err);
}

void log_ok (void)
{
    FILE * log_file = fopen ("log.txt", "a");
    
    MY_ASSERT (log_file == nullptr, "Log file cannot be opened.\n");
    fclose (log_file);
}

void print_code_err (int * error_number)
{
    printf ("code of error is | ");

    for (int i = 0; i < 32; i++)
    {
        printf ("%d", error_number[i]);

        if (!((i + 1) % 8))
            printf (" | ");
        if (!((i + 1) % 4))
            printf (" ");
    }
}

void logdump_hidden (unsigned char can_print, FILE * stack_log, stack_t * stk, const char * message, 
                     unsigned char is_err, unsigned char is_abort,  const char * call_func, 
                     const char* call_file, unsigned int call_line)
{
    MY_ASSERT (stk == nullptr, "There is no access to stk");
    size_t num = dump_call_num ();
    printf ("Number of the \"LOGDUMP\" function call is %zu\n", num);
    if (is_err)
    {
        fprintf (stack_log, LONG_LINE);
        fprintf (stack_log, "%s at %s(%d)\n", call_func, call_file, call_line);
        fprintf (stack_log, "Stack[%p](%d) \"%s\" was created in file %s in function %s(str %zu)\n", 
                    stk, is_err, stk->name ,stk->name_file_create, stk->name_func_create, stk->line_create);
        fprintf (stack_log, "%s\n", message);
        fprintf (stack_log, LONG_LINE);
        printf ("Please, check log file \"log.txt\".\n");
    }
    else
        fprintf (stack_log, "Everything is OK");
    if (can_print)
        stack_dump (*stk, stack_log);
    else
        printf ("Error information cannot be printed. Sorry.\n");
    if (is_abort && is_err)
        abort();
}

FILE * open_logfile (const char * name_logfile)
{
    FILE * log = fopen (name_logfile, "a");
    MY_ASSERT (log == nullptr, "There is no access to logfile");

    //setbuf (log, nullptr);

    return log;
}

void stk_ok (stack_t * stk, FILE * log)
{
    printf ("STK_OK\n");
    int sum_err = 0;
    if ((sum_err = struct_validator(stk, log)) != NOERR_STK)
    {
        printf ("Check log file \"log.txt\", you have some problems (with your head)\n");
        decoder (sum_err);
    }
}