#define NDEBUG_STK
#include "stack.h"


int main (void)
{
    stack_t stk = {};
    log_ok();
    FILE * logfile = fopen ("log.txt", "w");

    GET_INFO_STK();
    Ctor (&stk, 5);
    LOGDUMP(YES, logfile, &stk, "first attempt", NO);
    dump(stk, logfile);
    printf ("in main\n");
    Push (&stk, 4);
    printf ("in main\n");
    dump(stk, logfile);
    printf ("in main\n");
    //elem_t c = Pop (&stk);
    printf ("in main\n");
    SDtor (&stk);
    printf ("in main\n");
    dump(stk, logfile);

    return 0;
}

void Ctor (stack_t * stk, size_t capacity)
{
    stk -> n_memb = 0;
    stk -> capacity = capacity;
    stk -> stk_cnr_first  = STK_CNR_FRST;
    stk -> stk_cnr_second = STK_CNR_SCND;

    char * data = (char *) calloc (2*sizeof(cnr_t) + (stk->capacity)*sizeof(elem_t), sizeof(char));

    stk -> ptr_canary_data_first  = (cnr_t *) data;
    stk -> data = (elem_t *) ((char *)data + sizeof(cnr_t));
    stk -> ptr_canary_data_second = (cnr_t *) ((char *)data + sizeof(cnr_t) + (stk->capacity)*sizeof(elem_t));

    printf ("In Ctor: ");
    for (size_t i = 0; i < stk->capacity; i++)
    {
        *((elem_t*)((char *)stk->data + i * sizeof(elem_t))) = poison;
        printf ("%X, ", *((elem_t*)((char *)stk->data + i * sizeof(elem_t))));
    }
    printf ("\n");
    
} 


void Push (stack_t * stk, elem_t new_memb)
{
    stk->n_memb++;
    
    if (stk->n_memb > stk->capacity)
    {
        resize (stk, increase);
    }
    //check_err (); // проверить на n_memb > capacity, если это так, то возвращать ошибку
    
    stk->data[(stk->n_memb)-1] = new_memb;

    hash_sum (stk);
    STK_OK (stk);
}

elem_t Pop (stack_t * stk)
{
    stk->n_memb--;
    STK_OK (stk);
    MY_ASSERT (stk->n_memb < 0, "Error : there are no variables in the stack");
    elem_t pop = 0;
    if (stk->n_memb >= 0)
    {
        pop = (stk->data)[stk->n_memb];
        (stk->data)[stk->n_memb] = poison;
    }
    
    if (4*(stk->n_memb) <= stk->capacity)
    {
        resize(stk, reduce);
    }

    hash_sum(stk);
    STK_OK(stk);
    return pop;
}

void SDtor (stack_t * stk)
{
    stk->capacity = 0;
    stk->n_memb   = 0;
    stk->stk_cnr_second = 0;
    stk->hash_buf = 0;
    stk->hash_stk = 0;
    // *(stk->ptr_canary_data_first)  = 0;
    // *(stk->ptr_canary_data_second) = 0;

    free(stk->ptr_canary_data_first);
    // stk->data = nullptr;
    printf ("SDtor ok\n");
    stk->ptr_canary_data_first = nullptr;
    //free(stk);
    stk = nullptr;
}

void dump (stack_t stk, FILE * log_file)
{
    fprintf (log_file, "stk_cnr_first  = %llx\n", stk.stk_cnr_first);
    fprintf (log_file, "stk_cnr_second = %llx\n", stk.stk_cnr_second);
    fprintf (log_file, "\nArray in struct:\n");
    fwrite  (stk.data, sizeof(elem_t), sizeof(stk.data), log_file);
    fprintf (log_file, "\nNumber of members = %zu, capacity = %zu\n", stk.n_memb, stk.capacity);
    fprintf (log_file, "stk was created in file = %s, in func = %s, in strings = %d\n", stk.nameFileCreat, stk.nameFuncCreat, stk.lineCreat);
}

size_t dump_call_num (void)
{
    static size_t count = 0;
    count++;
    return count;
}

void * recalloc (void * memblock, size_t n_memb, size_t size_memb)
{
    if (n_memb == 0)
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

void resize (stack_t * stk, int mode)
{
    if (mode == increase)
    {
        stk->capacity *= RESIZE;
        char * ptr = (char *) recalloc (stk->ptr_canary_data_first, (stk->capacity)*sizeof(elem_t) + 2*sizeof(hash_t), sizeof(char));
        stk->ptr_canary_data_first = (cnr_t *) memcpy ((char *) ptr, (char *) stk->ptr_canary_data_first, (stk->capacity)*sizeof(elem_t) + 2*sizeof(hash_t));
        //check_err (); stk->data != nullptr
    }
    if (mode == reduce)
    {
        stk->capacity /= RESIZE;
        char * ptr = (char *) recalloc (stk->ptr_canary_data_first, (stk->capacity)*sizeof(elem_t) + 2*sizeof(hash_t), sizeof(char));
        stk->ptr_canary_data_first = (cnr_t *) memcpy ((char *) ptr, (char *) stk->ptr_canary_data_first, (stk->capacity)*sizeof(elem_t) + 2*sizeof(hash_t));
    }

    hash_sum (stk);
}

void hash_sum (stack_t * stk)
{
    stk -> hash_stk = calculateHash (stk, sizeof(stack_t));
    stk -> hash_buf = calculateHash (stk->data, sizeof(2*sizeof(hash_t) + (stk->capacity)*sizeof(elem_t)));
}

hash_t calculateHash (void * object, size_t byteSize)
{
    char * start_ptr = (char *) object;
    hash_t hash_sum = 0xDED60D;

    for (char i = 0; i < byteSize; i++)
    {
        hash_sum += i * start_ptr[i];
    }

    return hash_sum;
}

int struct_validator (stack_t * stk)
{
    log_ok ();

    printf ("value of ptr_stk_null = %d\n", ptr_stk_null);

    FILE * const log = fopen ("log.txt", "a");
    int err_sum = NOERR;

    if (stk == nullptr)
    {
        LOGDUMP(NO, log, stk, "The pointer to the stack is null", YES);
        fclose (log);
        err_sum |= ptr_stk_null;
    }
    if (stk->n_memb < 0)
    {
        LOGDUMP(NO, log, stk, "The number of members in the buffer is less then zero", YES);
        fclose (log);
        free (stk->data);
        err_sum |= bad_size;
    }
    if (stk->n_memb > stk->capacity)
    {
        LOGDUMP(NO, log, stk, "The number of members in the buffer is greater than its capacity", YES);
        fclose (log);
        free (stk->data);
        err_sum |= size_more_capac;
    }
    if (stk->data == nullptr)
    {
        LOGDUMP(NO, log, stk, "The pointer to buffer is null", YES);
        fclose (log);
        err_sum |= ptr_buf_null;
    }  
    if (stk->capacity <= 0)
    {
        LOGDUMP(NO, log, stk, "The capacity of the buffer is less than zero", YES);
        fclose (log);
        free (stk->data);
        err_sum |= bad_capacity;
    }
    if (stk->ptr_canary_data_first == nullptr)
    {
        LOGDUMP(NO, log, stk, "The pointer to first buffer canary is null", YES);
        fclose (log);
        err_sum |= bad_ptr_buf_can_frst;
    }
    if (stk->ptr_canary_data_second == nullptr)
    {
        LOGDUMP(NO, log, stk, "The pointer to second buffer canary is null", YES);
        fclose (log);
        err_sum |= bad_ptr_buf_can_scnd;
    }
    if (*(stk->ptr_canary_data_first) != BUF_CNR_FRST)
    {
        LOGDUMP(NO, log, stk, "The first buffer canary died", YES);
        fclose (log);
        err_sum |= bad_buf_can_frst;
    }
    if (*(stk->ptr_canary_data_second) != BUF_CNR_SCND)
    {
        LOGDUMP(NO, log, stk, "The second buffer canary died", YES);
        fclose (log);
        err_sum |= bad_buf_can_scnd;
    }
    if (stk->stk_cnr_first != STK_CNR_FRST)
    {
        LOGDUMP(NO, log, stk, "The first canary died", YES);
        fclose (log);
        err_sum |= bad_stk_can_frst;
    }
    if (stk->stk_cnr_second != STK_CNR_SCND)
    {
        LOGDUMP(NO, log, stk, "The second canary died", YES);
        fclose (log);
        err_sum |= bad_stk_can_scnd;
    }
    if (stk->hash_buf != calculateHash (stk->data, sizeof(2*sizeof(hash_t) + (stk->capacity)*sizeof(elem_t))))
    {
        LOGDUMP(NO, log, stk, "Hashsum of buffer isn't correct", YES);
        fclose (log);
        err_sum |= bad_buf_hash;
    }
    if (stk->hash_stk != calculateHash (stk, sizeof(stack_t)))
    {
        LOGDUMP(NO, log, stk, "Hashsum of stack isn't correct", YES);
        fclose (log);
        err_sum |= bad_stk_hash;
    }
    // for (int i = 0; i < (stk->capacity - stk->n_memb); i++)
    // {
    //     if (*((elem_t*)((char *)stk->data + (i + stk->n_memb)*sizeof(elem_t) + sizeof(cnr_t))))
    // }
    // LOGDUMP (YES, log, stk, "Hashsum of stack isn't correct", NO);
    fclose (log);
    
    return err_sum;
}

void decoder (int value_elem)
{
    int count_of_err[32] = {0};

    if ((value_elem & ptr_stk_null) == 2)
    {
        count_of_err[1] = 1;
    }
    if ((value_elem & ptr_buf_null) == 4)
    {
        count_of_err[2] = 1;
    }
    if ((value_elem & bad_capacity) == 8)
    {
        count_of_err[3] = 1;
    }
    if ((value_elem & bad_size) == 16)
    {
        count_of_err[4] = 1;
    }
    if ((value_elem & size_more_capac) == 32)
    {
        count_of_err[5] = 1;
    }
    if ((value_elem & bad_buf_can_frst) == 64)
    {
        count_of_err[6] = 1;
    }
    if ((value_elem & bad_buf_can_scnd) == 128)
    {  
        count_of_err[7] = 1;
    }
    if ((value_elem & bad_stk_can_frst) == 256)
    {
        count_of_err[8] = 1;
    }
    if ((value_elem & bad_stk_can_scnd) == 512)
    {
        count_of_err[9] = 1;
    }
    if ((value_elem & ptr_log_null) == 1024)
    {
        count_of_err[10] = 1;
    }
    if ((value_elem & bad_buf_hash) == 2048)
    {
        count_of_err[11] = 1;
    }
    if ((value_elem & bad_stk_hash) == 4096)
    {
        count_of_err[12] = 1;
    }
    if ((value_elem & bad_ptr_buf_hash) == 8192)
    {
        count_of_err[13] = 1;
    }
    if ((value_elem & bad_ptr_buf_can_frst) == 16384)
    {
        count_of_err[14] = 1;
    }
    if ((value_elem & bad_ptr_buf_can_scnd) == 32768)
    {
        count_of_err[15] = 1;
    }

    printf ("code of error is | ");

    for (int i = 0; i < 32; i++)
    {
        printf ("%d", count_of_err[i]);

        if (!((i + 1) % 8))
            printf (" | ");
        if (!((i + 1) % 4))
            printf (" ");
    }
}

void log_ok (void)
{
    FILE * log_file = fopen ("log.txt", "w");
    
    MY_ASSERT (log_file == nullptr, "Log file cannot be opened.\n");
    fclose (log_file);
}
