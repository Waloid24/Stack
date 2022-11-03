#define DEBUG_STK
#include "stack.h"


void stack_ctor (stack_t * stk, size_t capacity, const char * name_stk)
{
    stk -> n_memb = 0;
    stk -> capacity = capacity;
    stk -> min_capacity = capacity;
    stk -> name = name_stk;

    stk -> stk_cnr_second = STK_CNR_SCND;

    char * data = (char *) calloc (2*sizeof(cnr_t) + (stk->capacity)*sizeof(elem_t), sizeof(char));

    stk -> ptr_canary_data_first  = (cnr_t *) data;
    stk -> data = (elem_t *) ((char *)data + sizeof(cnr_t));
    stk -> ptr_canary_data_second = (cnr_t *) ((char *)data + sizeof(cnr_t) + (stk->capacity)*sizeof(elem_t));
    *(stk -> ptr_canary_data_second) = BUF_CNR_SCND;

    //printf ("In Ctor: ");
    for (size_t i = 0; i < stk->capacity; i++)
    {
        *((elem_t*)((char *)stk->data + i * sizeof(elem_t))) = POISON;
        //printf ("%X, ", *((elem_t*)((char *)stk->data + i * sizeof(elem_t))));
    }
    //printf ("\n");

    *(stk -> ptr_canary_data_first) = calculate_hash (stk->data, sizeof(sizeof(hash_t) + (stk->capacity)*sizeof(elem_t)));
    stk -> stk_cnr_first = calculate_hash ((char*)stk + (char) sizeof (cnr_t), sizeof(stack_t)-sizeof(cnr_t));
    
} 


void stack_push (stack_t * stk, elem_t new_memb)
{
    MY_ASSERT (stk == nullptr, "No stack access");
    FILE * logfile = fopen ("log.txt", "a");
    
    if (stk->n_memb >= stk->capacity)
    {
        stack_resize (stk, increase);
    }
    stk->data[stk->n_memb] = new_memb;
    stk->n_memb++;

    stack_hash_sum (stk);
    STK_OK (stk);
    fclose (logfile);
}

elem_t stack_pop (stack_t * stk)
{
    stk->n_memb--;

    stack_hash_sum(stk);
    // printf ("IN PP\n");
    // STK_OK (stk);
    // printf ("IN POOOP\n");
    MY_ASSERT (stk->n_memb < 0, "Error : there are no variables in the stack"); 

    elem_t pop = 0;
    if (stk->n_memb >= 0)
    {
        pop = (stk->data)[stk->n_memb];
        (stk->data)[stk->n_memb] = POISON;
    }
    
    if (THRESHOLD_RATIO*(stk->n_memb) <= stk->capacity && (stk->capacity/RESIZE) >= (stk -> min_capacity)) 
    {
        stack_resize(stk, reduce);
    }

    stack_hash_sum(stk);
    STK_OK(stk);

    return pop;
}

void stack_Dtor (stack_t * stk)
{
    stk->capacity = 0;
    stk->n_memb   = 0;
    stk->stk_cnr_second = 0;
    // *(stk->ptr_canary_data_first)  = 0;
    // *(stk->ptr_canary_data_second) = 0;

    free(stk->ptr_canary_data_first);
    // stk->data = nullptr; // ToDo: занули все поля
    //printf ("stack_Dtor ok\n");
    stk->ptr_canary_data_first = nullptr;
    //free(stk);
    stk = nullptr;
}

void stack_dump (stack_t stk, FILE * log_file) // ToDo: ptr instead value
{
    MY_ASSERT (log_file == nullptr, "logfile is close");
    if (stk.capacity == 0)
    {
        printf ("stk.capacity = 0, maybe you have used the function Dtor\n");
        abort();
    }

    //ToDo: библиотека не длолжна падать. Обрабатываем ошибку и выводим что-то в файл, не роняя выполнение.
    //printf ("1\n");
    fprintf (log_file, LONG_LINE);
    fprintf (log_file, "Hashsum of stack  = %llx\n", stk.stk_cnr_first);
    //printf ("2\n");
    fprintf (log_file, "stk_cnr_second = %llx\n", stk.stk_cnr_second);
    //printf ("3\n");
    fprintf (log_file, "\nArray in struct:\n");
    //printf ("4\n");

    fprintf (log_file, "Hashsum of data: %llX\n", *(stk.ptr_canary_data_first));
    fprintf (log_file, "In data:");
    for (size_t i = 0; i < stk.capacity; i++)
    {
        fprintf (log_file, "%X, ", *((elem_t*)((char *)stk.data + i * sizeof(elem_t))));
    }
    fprintf (log_file, "\n");
    fprintf (log_file, "Second canary in data: %llX\n", *(stk.ptr_canary_data_second));

    
    //printf ("5\n");
    fprintf (log_file, "\nNumber of members = %zu, capacity = %zu\n", stk.n_memb, stk.capacity);
    fprintf (log_file, "stk was created in file = %s, in func = %s, in strings = %d\n", stk.nameFileCreat, stk.nameFuncCreat, stk.lineCreat);
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
    if (mode == increase)
    {
        stk->capacity *= RESIZE;
        cnr_t temp     = *(stk->ptr_canary_data_second);
        //printf ("in resize before recalloc stk->ptr_canary_data_second = %p\n", stk->ptr_canary_data_second);
        //printf ("The value of stk->ptr_canary_data_second BEFORE resize = %llX\n", *(stk->ptr_canary_data_second));

        char * ptr = (char *) stack_recalloc (stk->ptr_canary_data_first, (stk->capacity)*sizeof(elem_t) + 2*sizeof(hash_t), sizeof(char));

        stk->ptr_canary_data_first     = (hash_t *) ptr;
        stk->data                      = (elem_t *)((char *)stk->ptr_canary_data_first + sizeof (hash_t));
        stk->ptr_canary_data_second    = (cnr_t *)((char *)stk->ptr_canary_data_first + (char)((stk->capacity)*sizeof(elem_t)) + sizeof(hash_t));
        *(stk->ptr_canary_data_second) = temp;

        MY_ASSERT (stk->ptr_canary_data_first == nullptr, "New pointer after resize is nullptr");

        for (int i = 0; i < (stk->capacity - stk->n_memb); i++)
        {
            *((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb) = POISON;
        }

        //stk->ptr_canary_data_first = (cnr_t *) ptr;
        //printf ("in resize after recalloc stk->ptr_canary_data_second = %p\n", stk->ptr_canary_data_second);
        //printf ("The value of stk->ptr_canary_data_second AFTER resize = %llX\n", *(stk->ptr_canary_data_second));
        //check_err (); stk->data != nullptr
    }
    else if (mode == reduce)
    {
        stk->capacity /= RESIZE;
        cnr_t temp = *(stk->ptr_canary_data_second);

        //printf ("in resize before recalloc stk->ptr_canary_data_second = %p\n", stk->ptr_canary_data_second);
        //printf ("The value of stk->ptr_canary_data_second BEFORE resize = %llX\n", *(stk->ptr_canary_data_second));

        char * ptr = (char *) stack_recalloc (stk->ptr_canary_data_first, (stk->capacity)*sizeof(elem_t) + 2*sizeof(hash_t), sizeof(char));

        stk->ptr_canary_data_first     = (hash_t *) ptr;
        stk->data                      = (elem_t *)((char *)stk->ptr_canary_data_first + sizeof (hash_t));
        stk->ptr_canary_data_second    = (cnr_t  *)((char *)stk->ptr_canary_data_first + (char)((stk->capacity)*sizeof(elem_t)) + sizeof(hash_t));
        *(stk->ptr_canary_data_second) = temp;

        MY_ASSERT (stk->ptr_canary_data_first == nullptr, "New pointer after resize is nullptr");

        for (int i = 0; i < (stk->capacity - stk->n_memb); i++)
        {
            *((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb) = POISON;
        }

        //stk->ptr_canary_data_first = (cnr_t *) ptr;
        //printf ("in resize after recalloc stk->ptr_canary_data_second = %p\n", stk->ptr_canary_data_second);
        //printf ("The value of stk->ptr_canary_data_second AFTER resize = %llX\n", *(stk->ptr_canary_data_second));
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

    *(stk -> ptr_canary_data_first) = calculate_hash (stk->data, sizeof(sizeof(hash_t) + (stk->capacity)*sizeof(elem_t)));
    //printf ("Hashsum of data = %llX\n", *(stk -> ptr_canary_data_first));
    stk -> stk_cnr_first = calculate_hash ((char*)stk + (char) sizeof (cnr_t), sizeof(stack_t)-sizeof(cnr_t));
    //printf ("Hashsum of stk = %llX\n", stk -> stk_cnr_first);
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

int struct_validator (stack_t * stk)
{
    log_ok ();

    FILE * const log = fopen ("log.txt", "a");
    int err_sum = NOERR;
    //printf ("inside str_validator\n");
    if (stk == nullptr)
    {
        //printf ("stk == nullptr\n");
        LOGDUMP(NO, log, stk, "The pointer to the stack is null", YES);
        fclose (log);
        err_sum |= ptr_stk_null;
    }
    if (stk->n_memb < 0) // ToDo: смысла нет
    {
        //printf ("stk->n_memb < 0\n");
        LOGDUMP(YES, log, stk, "The number of members in the buffer is less then zero", YES);
        fclose (log);
        free (stk->data);
        err_sum |= bad_size;
    }
    if (stk->n_memb > stk->capacity)
    {
        //printf ("stk->n_memb > stk->capacity\n");
        LOGDUMP(YES, log, stk, "The number of members in the buffer is greater than its capacity", YES);
        fclose (log);
        free (stk->data);
        err_sum |= size_more_capac;
    }
    if (stk->data == nullptr)
    {
        //printf ("stk->data == nullptr\n");
        LOGDUMP(NO, log, stk, "The pointer to buffer is null", YES);
        fclose (log);
        err_sum |= ptr_buf_null;
    }  
    if (stk->capacity <= 0)
    {
        //printf ("stk->capacity <= 0\n");
        LOGDUMP(YES, log, stk, "The capacity of the buffer is less than zero", YES);
        fclose (log);
        free (stk->data);
        err_sum |= bad_capacity;
    }
    if (stk->ptr_canary_data_first == nullptr)
    {
        //printf ("stk->ptr_canary_data_first == nullptr\n");
        LOGDUMP(NO, log, stk, "The pointer to hash-sum of buffer is null", YES);
        fclose (log);
        err_sum |= bad_ptr_buf_hash;
    }
    if (stk->ptr_canary_data_second == nullptr)
    {
        //printf ("stk->ptr_canary_data_second == nullptr\n");
        LOGDUMP(NO, log, stk, "The pointer to second buffer canary is null", YES);
        fclose (log);
        err_sum |= bad_ptr_buf_can_scnd;
    }
    if (*(stk->ptr_canary_data_first) != calculate_hash (stk->data, sizeof(sizeof(hash_t) + (stk->capacity)*sizeof(elem_t))))
    {
        //printf ("*(stk->ptr_canary_data_first) = %llX\n", *(stk->ptr_canary_data_first));
        //printf ("stk->hash_buf != calculate_hash\n");
        LOGDUMP(YES, log, stk, "Hashsum of buffer isn't correct", YES);
        fclose (log);
        err_sum |= bad_buf_hash;
    }
    if (*(stk->ptr_canary_data_second) != BUF_CNR_SCND)
    {
        //printf ("*(stk->ptr_canary_data_second) != BUF_CNR_SCND\n");
        //printf ("*(stk->ptr_canary_data_second) = %llX\n", *(stk->ptr_canary_data_second));
        LOGDUMP(YES, log, stk, "The second buffer canary died", YES);
        fclose (log);
        err_sum |= bad_buf_can_scnd;
    }
    if (stk->stk_cnr_first != calculate_hash ((char*)stk + (char) sizeof (cnr_t), sizeof(stack_t)-sizeof(cnr_t)))
    {
        //printf ("stk->stk_cnr_first == %llX", calculate_hash ((char*)stk + (char) sizeof (cnr_t), sizeof(stack_t)-sizeof(cnr_t)));
        //printf ("stk->stk_cnr_first != calculate_hash\n");
        LOGDUMP(YES, log, stk, "Hashsum of stack isn't correct", YES);
        fclose (log);
        err_sum |= bad_stk_hash;
    }
    if (stk->stk_cnr_second != STK_CNR_SCND)
    {
        //printf ("stk->stk_cnr_second != STK_CNR_SCND\n");
        LOGDUMP(YES, log, stk, "The second canary died", YES);
        fclose (log);
        err_sum |= bad_stk_can_scnd;
    }
    for (int i = 0; i < (stk->capacity - stk->n_memb); i++)
    {
        if (*((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb) != POISON)
        {
            //printf ("*((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb) = %d\n", *((elem_t *)stk->data + (elem_t)i + (elem_t)stk->n_memb));
            //printf ("Poison isn't poison! ^_^\n");
            LOGDUMP(YES, log, stk, "The value of the poison cell has been changed", YES);
            fclose (log);
            err_sum |= bad_poison;
        }
    }
    // LOGDUMP (YES, log, stk, "Hashsum of stack isn't correct", NO);
    
    return err_sum;
}
// использовать только перед struct_validator, ведь только там он используется, потом undef
void decoder (int value_elem)
{
    int count_of_err[32] = {0};

    if ((value_elem & ptr_stk_null)         == 2) // == ptr_stk_null????
    {
        count_of_err[1] = 1;
    }
    if ((value_elem & ptr_buf_null)         == 4)
    {
        count_of_err[2] = 1;
    }
    if ((value_elem & ptr_log_null)         == 8)
    {
        count_of_err[3] = 1;
    }
    if ((value_elem & bad_capacity)         == 16)
    {
        count_of_err[4] = 1;
    }
    if ((value_elem & bad_size)             == 32)
    {
        count_of_err[5] = 1;
    }
    if ((value_elem & size_more_capac)      == 64)
    {
        count_of_err[6] = 1;
    }
    if ((value_elem & bad_buf_can_scnd)     == 128)
    {  
        count_of_err[7] = 1;
    }
    if ((value_elem & bad_stk_can_scnd)     == 256)
    {
        count_of_err[8] = 1;
    }
    
    if ((value_elem & bad_buf_hash)         == 512)
    {
        count_of_err[9] = 1;
    }
    if ((value_elem & bad_stk_hash)         == 1024)
    {
        count_of_err[10] = 1;
    }
    if ((value_elem & bad_ptr_buf_hash)     == 2048)
    {
        count_of_err[11] = 1;
    }
    if ((value_elem & bad_ptr_buf_can_scnd) == 4096)
    {
        count_of_err[12] = 1;
    }
    if ((value_elem & bad_poison)           == 8192)
    {
        count_of_err[13] = 1;
    }

    print_code_err (count_of_err);
}

void log_ok (void)
{
    FILE * log_file = fopen ("log.txt", "w");
    
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
        fprintf (stack_log, "Stack[%p](%d) \"%s\" was created in file %s in function %s(str %d)\n", 
                    stk, is_err, stk->name ,stk->nameFileCreat, stk->nameFuncCreat, stk->lineCreat);
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

