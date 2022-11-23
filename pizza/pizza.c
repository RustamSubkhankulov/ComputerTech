#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <sys/sem.h>
#include <stdbool.h>

//=========================================================

#define ERR(action)                                         \
                                                            \
    do                                                      \
    {                                                       \
                                                            \
        fprintf(stderr, "File: %s ",  __FILE__);            \
        fprintf(stderr, "Line: %d\n", __LINE__);            \
        action;                                             \
                                                            \
    } while (0);                                
    

//=========================================================

struct Cook_convey
{
    int sem_id;

    unsigned sem_ingredient_access;
    unsigned sem_ingredient_num;

    unsigned sem_convey;

    unsigned next_convey_sem_access;

    char ingr;
};

//=========================================================

static int check_terminate (int sem_id);

static int sem_v           (int sem_id, unsigned sem_num);

static int sem_p           (int sem_id, unsigned sem_num);

static int sem_oper        (int sem_id, unsigned short sem_num, short sem_op, short sem_flg);

static int cook_ingredients(int sem_id, char* ingredients, unsigned* ingredient_sems_access,
                                                           unsigned* ingredient_sems_num, char** storage);

static int client          (int sem_id, char* ready);

static int cook_convey     (const struct Cook_convey* Cook_convey, 
                            unsigned len, unsigned count, char* storage, char* workplace, bool* wake_up_next);

static int chief           (int sem_id, char** convey, char* ready, char* empty, bool** wake_up_next_arr);

//=========================================================

static const unsigned Pizza_num = 10;

static const unsigned Ingredient_low  = 5;
static const unsigned Ingredient_high = 10;

static const unsigned Ingredients_num = 4;

static const unsigned Product_len = strlen("pizza");

static const unsigned P_convey_len = 1;
static const unsigned I_convey_len = 2;
static const unsigned Z_convey_len = 4;
static const unsigned A_convey_len = 5;

//---------------------------------------------------------

// semaphores

static const unsigned Sem_num = 16;

static const unsigned P_ingredient_access = 0;
static const unsigned I_ingredient_access = 1;
static const unsigned Z_ingredient_access = 2;
static const unsigned A_ingredient_access = 3;

static const unsigned P_ingredient_num = 4;
static const unsigned I_ingredient_num = 5;
static const unsigned Z_ingredient_num = 6;
static const unsigned A_ingredient_num = 7;

static const unsigned P_convey_sem  = 8;
static const unsigned I_convey_sem  = 9;
static const unsigned Z_convey_sem  = 10;
static const unsigned A_convey_sem  = 11; 

static const unsigned Ready_sem     = 12;

static const unsigned Terminate_sem = 13;

static const unsigned Client_sem    = 14;
static const unsigned Chief_sem     = 15;

//=========================================================

int main()
{
    // prepare all for work

    size_t size = sizeof(char) * (4 * Ingredient_high + 6 * strlen("pizza") + 5 * sizeof(char) + 4 * sizeof(bool));

    int shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | 0700);
    void* addr = shmat(shm_id, NULL, 0);

    memset(addr, 0, size);

    char* empty = (char*) addr;
    char* ready = (char*) (addr + Product_len * 5);

    char* convey[] = {((char*) addr + Product_len),
                      ((char*) addr + Product_len * 2),
                      ((char*) addr + Product_len * 3),
                      ((char*) addr + Product_len * 4)};

    char     ingredients      [] = {'p'  , 'i'  , 'z'  , 'a'};
    unsigned ingredients_count[] = {1, 1, 2, 1};

    int ingredient_sems_access[] = {P_ingredient_access, 
                                    I_ingredient_access, 
                                    Z_ingredient_access, 
                                    A_ingredient_access};

    unsigned ingredient_sems_num[] = {P_ingredient_num, 
                                      I_ingredient_num, 
                                      Z_ingredient_num, 
                                      A_ingredient_num};


    unsigned convey_sem_nums[] = {P_convey_sem, 
                                  I_convey_sem, 
                                  Z_convey_sem, 
                                  A_convey_sem,
                                  Chief_sem};

    unsigned convey_lens[] = {P_convey_len,
                              I_convey_len,
                              Z_convey_len,
                              A_convey_len};

    char* cook_storage[] = {((char*) addr + Product_len * 6 + 1),
                            ((char*) addr + Product_len * 6 + Ingredient_high + 2),
                            ((char*) addr + Product_len * 6 + Ingredient_high * 2 + 3),
                            ((char*) addr + Product_len * 6 + Ingredient_high * 3 + 4)};

    bool* wake_up_next_arr_addr = (bool*)((char*) addr + Product_len * 6 + Ingredient_high * 4 + 5);
    bool* wake_up_next[] = {wake_up_next_arr_addr,
                            wake_up_next_arr_addr + sizeof(bool),
                            wake_up_next_arr_addr + sizeof(bool) * 2,
                            wake_up_next_arr_addr + sizeof(bool) * 3 };
    for (unsigned iter = 0; iter < sizeof(wake_up_next) / sizeof(bool*); iter++)
    {
        *(wake_up_next[iter]) = true;
    }

    int sem_id = semget(IPC_PRIVATE, Sem_num, IPC_CREAT | IPC_EXCL | 0700);
    if (sem_id == -1) 
    {
        ERR(fprintf(stderr, "semget() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    pid_t pid = 0;

    // set semaphores to their initial values

    struct sembuf initial[] = {{.sem_num = P_ingredient_access, .sem_op = +1, .sem_flg = 0},
                               {.sem_num = I_ingredient_access, .sem_op = +1, .sem_flg = 0},
                               {.sem_num = Z_ingredient_access, .sem_op = +1, .sem_flg = 0},
                               {.sem_num = A_ingredient_access, .sem_op = +1, .sem_flg = 0},
                               {.sem_num = P_convey_sem,        .sem_op = +1, .sem_flg = 0},
                            //    {.sem_num = I_convey_sem,        .sem_op = +1, .sem_flg = 0},
                            //    {.sem_num = Z_convey_sem,        .sem_op = +1, .sem_flg = 0},
                            //    {.sem_num = A_convey_sem,        .sem_op = +1, .sem_flg = 0}};
                               {.sem_num = Ready_sem,           .sem_op = +1, .sem_flg = 0}};

    int err = semop(sem_id, initial, sizeof(initial) / sizeof(struct sembuf));
    if (err != 0)
    {
        ERR(fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    // run child processes

    for (unsigned iter = 0; iter < Ingredients_num; iter++)
    {
        pid = fork();

        if (pid == 0)
        {
            struct Cook_convey cur = {.sem_id = sem_id, .sem_ingredient_access      = ingredient_sems_access[iter],
                                                            .sem_ingredient_num     = ingredient_sems_num   [iter], 
                                                            .sem_convey             = convey_sem_nums       [iter],
                                                            .ingr                   = ingredients           [iter],
                                                            .next_convey_sem_access = convey_sem_nums       [iter + 1]};

            return cook_convey(&cur, convey_lens[iter], ingredients_count[iter], cook_storage[iter], convey[iter], wake_up_next[iter]);
        }
        else if (pid == -1)
        {
            ERR(fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno)));
            return -errno;
        }
    }

    pid = fork();
    if (pid == 0)
    {
        return cook_ingredients(sem_id, ingredients, ingredient_sems_access, 
                                                     ingredient_sems_num, cook_storage);
    }
    else if (pid == -1)
    {
        ERR(fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    pid = fork();
    if (pid == 0)
    {
        return chief(sem_id, convey, ready, empty, wake_up_next);
    }
    else if (pid == -1)
    {
        ERR(fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    pid = fork();
    if (pid == 0)
    {
        return client(sem_id, ready);
    }
    else if (pid == -1)
    {
        ERR(fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    // wait all child processes to terminate

    for (unsigned iter = 0; iter < Ingredients_num + 3; iter++)
    {   
        pid_t term = wait(NULL);
        if (term == -1)
        {
            ERR(fprintf(stderr, "wait() syscall failed: %s \n", strerror(errno)));
            return -errno;
        }
    }

    // deattach and/or delete semaphores and shared memory

    err = semctl(sem_id, 1, IPC_RMID);
    if (err == -1)
    {
        ERR(fprintf(stderr, "semctl() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = shmdt(addr);
    if (err == -1)
    {
        ERR(fprintf(stderr, "shmdt() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    err = shmctl(shm_id, IPC_RMID, NULL);
    if (err == -1)
    {
        ERR(fprintf(stderr, "shmctl() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static int check_terminate(int sem_id)
{
    struct sembuf check = {.sem_num = Terminate_sem, .sem_op = -1, .sem_flg = IPC_NOWAIT};
    
    int err = semop(sem_id, &check, 1);
    if (err != 0 && errno == EAGAIN)
    {   
        return 0;
    }
    else if (err != 0 && errno != EAGAIN)
    {
        ERR(fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno)));
        return -errno;
    } 
        
    return 1;
}

//---------------------------------------------------------

static int sem_v(int sem_id, unsigned sem_num)
{
    struct sembuf op_v = {.sem_num = sem_num, .sem_op = +1, .sem_flg = 0};
    
    int err = semop(sem_id, &op_v, 1);
    if (err != 0)
    {
        ERR(fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    return err;
}

//---------------------------------------------------------

static int sem_p(int sem_id, unsigned sem_num)
{
    struct sembuf op_p = {.sem_num = sem_num, .sem_op = -1, .sem_flg = 0};

    int err = semop(sem_id, &op_p, 1);
    if (err != 0)
    {
        ERR(fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    return err;
}

//---------------------------------------------------------

static int sem_oper(int sem_id, unsigned short sem_num, short sem_op, short sem_flg)
{
    struct sembuf op = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = sem_flg};

    int err = semop(sem_id, &op, 1);
    if (err != 0)
    {
        ERR(fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno)));
        return -errno;
    }

    return err;
}

//---------------------------------------------------------

static int cook_ingredients(int sem_id, char* ingredients, unsigned* ingredient_sems_access,
                                                           unsigned* ingredient_sems_num, char** storage)
{
    assert(ingredients);
    assert(storage);
    assert(ingredient_sems_num);
    assert(ingredient_sems_access);

    fprintf(stderr, "INGR: Ingredients cook started \n");

    while (1)
    {
        fprintf(stderr, "INGR: iteration \n");

        int term = 0;
        if ((term = check_terminate(sem_id)) == 1)
        {
            fprintf(stderr, "INGR: Ingredients cook terminated normally \n");
            return 0;
        }
        else if (term != 0)
        {
            fprintf(stderr, "INGR: Ingredients cook terminated with error \n");
            return term;
        }

        for (unsigned iter = 0; iter < Ingredients_num; iter++)
        {
            int err = sem_p(sem_id, ingredient_sems_access[iter]);
            if (err != 0)
            {
                fprintf(stderr, "INGR: Ingredients cook terminated with error \n");
                return err;
            }

            fprintf(stderr, "INGR: Ingredients cook took access for '%c' \n", ingredients[iter]);

            int ingredient_amount = strlen(storage[iter]);

            fprintf(stderr, "INGR: Current amount of '%c' is %d \n", ingredients[iter], ingredient_amount);
            fprintf(stderr, "INGR: Ingredients string: |%s| \n", storage[iter]);

            if (ingredient_amount <= Ingredient_low)
            {
                fprintf(stderr, "INGR: Not enough of '%c'. Started cooking ingredients \n", ingredients[iter]);

                int cooked = 0;
                char ingredient = ingredients[iter];
                char src[2] = {ingredient, '\0'};

                while(ingredient_amount + cooked < Ingredient_high)
                {
                    strcat(storage[iter], src);
                    cooked++;
                }

                fprintf(stderr, "INGR: Cooked %d '%c'ingredients \n", cooked, ingredient);
                fprintf(stderr, "INGR: Ingredients string after cooking: |%s| \n", storage[iter]);

                struct sembuf oper = {.sem_num = ingredient_sems_num[iter], .sem_op = cooked, .sem_flg = 0};
                err = semop(sem_id, &oper, 1);
                if (err != 0)
                {
                    ERR(fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno)));
                    fprintf(stderr, "INGR: Ingredients cook terminated with error \n");
                    return -errno;
                }
            }

            err = sem_v(sem_id, ingredient_sems_access[iter]);
            if (err != 0)
            {
                fprintf(stderr, "INGR: Ingredients cook terminated with error \n");
                return err;
            }
        }
        
        usleep(100);
    }

    return 0;
}

//---------------------------------------------------------

static int client(int sem_id, char* ready)
{
    assert(ready);

    fprintf(stderr, "CLNT: Client started \n");

    unsigned pizza_count = 0;

    while (pizza_count < Pizza_num)
    {
        fprintf(stderr, "CLNT: iteration \n");

        int err = sem_p(sem_id, Client_sem);
        if (err != 0)
        {
            fprintf(stderr, "CLNT: Client terminated with error \n");
            return err;
        }

        fprintf(stderr, "CLNT: Chief told client that pizza is ready \n");

        fprintf(stderr, "CLNT: Cur pizza count: %d \n", pizza_count);

        err = sem_p(sem_id, Ready_sem);
        if (err != 0)
        {
            fprintf(stderr, "CLNT: Client terminated with error \n");
            return err;
        }

        fprintf(stderr, "CLNT: Client checks ready pizza: |%s| \n", ready);

        if (strcmp(ready, "pizza") == 0)
        {
            fprintf(stderr, "CLNT: Client took pizza \n");

            pizza_count++;
            *ready = '\0';

            fprintf(stderr, "CLNT: Ready pizza now is |%s| \n", ready);
        }

        err = sem_v(sem_id, Ready_sem);
        if (err != 0)
        {
            fprintf(stderr, "CLNT: Client terminated with error \n");
            return err;
        }
    }

    fprintf(stderr, "CLNT: Client is satisfied. Terminating \n");
    return sem_oper(sem_id, Terminate_sem, Ingredients_num + 2, 0);
}

//---------------------------------------------------------

static int cook_convey(const struct Cook_convey* Cook_convey, 
                       unsigned len, unsigned count, char* storage, char* workplace, bool* wake_up_next)
{
    assert(Cook_convey);
    assert(storage);
    assert(workplace);
    assert(wake_up_next);
   
    char ingredient = Cook_convey->ingr;

    fprintf(stderr, "CNVY: (%c) Convey cook started \n", ingredient);

    while (1)
    {
        // fprintf(stderr, "CNVY: (%c) iteration \n", ingredient);

        int err = sem_p(Cook_convey->sem_id, Cook_convey->sem_convey);
        if (err != 0)
        {
            fprintf(stderr, "CNVY: (%c) Convey cook terminated with error \n", ingredient);
            return err;
        }

        size_t workplace_len = strlen(workplace);

        if ((workplace_len == 0 && ingredient != 'p') 
         || (workplace_len != 0 && workplace[workplace_len - 1] == ingredient))
        {
            fprintf(stderr, "CNVY: (%c) Len is %lu Last sym is %c; No need to cook, skips \n", ingredient, workplace_len, (workplace_len)? workplace[workplace_len - 1] : 'f');            
            goto free_workplace;
        }

        fprintf(stderr, "CNVY: (%c) Workplace: |%s| \n", ingredient, workplace);
        fprintf(stderr, "CNVY: (%c) Need to add ingredients \n", ingredient);

        err = sem_p(Cook_convey->sem_id, Cook_convey->sem_ingredient_access);
        if (err != 0)
        {
            fprintf(stderr, "CNVY: (%c) Convey cook terminated with error \n", ingredient);
            return err;
        }

        struct sembuf operation = {.sem_num = Cook_convey->sem_ingredient_num, 
                                   .sem_op  = -count, 
                                   .sem_flg = IPC_NOWAIT};

        int result = 0;
        
        while(1)
        {
            fprintf(stderr, "CNVY: (%c) try and lock num of ingredients \n", ingredient);

            result = semop(Cook_convey->sem_id, &operation, 1);
            if (result == 0 || (result != 0 && errno != EAGAIN))
                break;

            fprintf(stderr, "CNVY: (%c) Not enough ingredients, wait for ingredients cook (sleep)\n", ingredient);

            err = sem_v(Cook_convey->sem_id, Cook_convey->sem_ingredient_access);
            if (err != 0)
            {
                fprintf(stderr, "CNVY: (%c) Convey cook terminated with error \n", ingredient);
                return err;
            }

            usleep(50);

            err = sem_p(Cook_convey->sem_id, Cook_convey->sem_ingredient_access);
            if (err != 0)
            {
                fprintf(stderr, "CNVY: (%c) Convey cook terminated with error \n", ingredient);
                return err;
            }
        }

        if (result != 0)
        {
            ERR(fprintf(stderr, "semop() syscall failed: %s %d\n", strerror(errno), errno));
            fprintf(stderr, "CNVY: (%c) Cook convey terminated with error \n", ingredient);
            return -errno;
        }

            fprintf(stderr, "CNVY: (%c) Enough ingredients in storage \n", ingredient);

        fprintf(stderr, "CNVY: (%c) Took ingredients from storage \n", ingredient);
        fprintf(stderr, "CNVY: (%c) before: storage |%s| workplace |%s| \n", ingredient, storage, workplace);

        for (unsigned iter = 0; iter < count; iter++)
        {
            char ingr = storage[strlen(storage) - 1];
            storage[strlen(storage) - 1] = '\0';

            strncat(workplace, &ingr, 1);
        }

        fprintf(stderr, "CNVY: (%c) after: storage |%s| workplace |%s| \n", ingredient, storage, workplace);

        err = sem_v(Cook_convey->sem_id, Cook_convey->sem_ingredient_access);
        if (err != 0)
        {
            fprintf(stderr, "CNVY: (%c) Convey cook terminated with error \n", ingredient);
            return err;
        }

    free_workplace:

        if (*wake_up_next == true)
        {
            err = sem_v(Cook_convey->sem_id, Cook_convey->next_convey_sem_access);
            if (err != 0)
            {
                fprintf(stderr, "CNVY: (%c) Convey cook terminated with error \n", ingredient);
                return err;
            }

            fprintf(stderr, "CNVY: (%c) Convey cook woke up next \n", ingredient);
            *wake_up_next = false;
        }

        err = sem_v(Cook_convey->sem_id, Cook_convey->sem_convey);
        if (err != 0)
        {
            fprintf(stderr, "CNVY: (%c) Convey cook terminated with error \n", ingredient);
            return err;
        }

        int term = 0;
        if ((term = check_terminate(Cook_convey->sem_id)) == 1)
        {
            fprintf(stderr, "CNVY: (%c) Convey cook terminated normally \n", ingredient);
            return 0;
        }
        else if (term != 0)
        {
            fprintf(stderr, "CNVY: (%c) Convey cook terminated with error \n", ingredient);
            return term;
        }

        usleep(100);
    }

    return 0;
}

//---------------------------------------------------------

static int chief(int sem_id, char** convey, char* ready, char* empty, bool** wake_up_next_arr)
{
    assert(convey);
    assert(ready);
    assert(empty);
    assert(wake_up_next_arr);

    fprintf(stderr, "CHIF: Chief started \n");

    while (1)
    {
        fprintf(stderr, "CHIF: iteration \n");

        int err = sem_p(sem_id, Chief_sem);
        if (err != 0)
        {
            fprintf(stderr, "CHIF: Chief terminated with error \n");
            return err;
        }

        int term = 0;
        if ((term = check_terminate(sem_id)) == 1)
        {
            fprintf(stderr, "CHIF: Chief terminated normally \n");
            return 0;
        }
        else if (term != 0)
        {
            fprintf(stderr, "CHIF: Chief terminated with error \n");
            return term;
        }

        struct sembuf ops[] = {{.sem_num = P_convey_sem, .sem_op = -1, .sem_flg = 0},
                               {.sem_num = I_convey_sem, .sem_op = -1, .sem_flg = 0},
                               {.sem_num = Z_convey_sem, .sem_op = -1, .sem_flg = 0},
                               {.sem_num = A_convey_sem, .sem_op = -1, .sem_flg = 0},
                               {.sem_num = Ready_sem,    .sem_op = -1, .sem_flg = 0}};

        err = semop(sem_id, ops, sizeof(ops) / sizeof(struct sembuf));
        if (err != 0)
        {
            ERR(fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno)));
            fprintf(stderr, "CHIF: Chief terminated with error \n");
            return -errno;
        }

        fprintf(stderr, "CHIF: Chief checking convey \n");

        fprintf(stderr, "CHIF: Empty    |%s| \n", empty);
        fprintf(stderr, "CHIF: P_convey |%s| \n", convey[0]);
        fprintf(stderr, "CHIF: I_convey |%s| \n", convey[1]);
        fprintf(stderr, "CHIF: Z_convey |%s| \n", convey[2]);
        fprintf(stderr, "CHIF: A_convey |%s| \n", convey[3]);
        fprintf(stderr, "CHIF: Ready    |%s| \n", ready);

        if (((strlen(convey[0]) != P_convey_len)
          && (strlen(convey[1]) != I_convey_len || strlen(convey[1]) != 0) 
          && (strlen(convey[2]) != Z_convey_len || strlen(convey[2]) != 0)
          && (strlen(convey[3]) != A_convey_len || strlen(convey[3]) != 0))
           || strlen(ready) != 0)
        {
            fprintf(stderr, "CHIF: Convey is not ready, skips \n");
            
            for (unsigned iter = 0; iter < sizeof(ops) / sizeof(struct sembuf); iter++)
            {
                ops[iter].sem_op = +1;
            }

            err = semop(sem_id, ops, sizeof(ops) / sizeof(struct sembuf));
            if (err != 0)
            {
                fprintf(stderr, "CHIF: Chief terminated with error \n");
                return err;
            }
        }

        // move convey

        fprintf(stderr, "CHIF: Convey is ready, moving \n");

        err = sem_v(sem_id, Ready_sem);
        if (err != 0)
        {
            fprintf(stderr, "CHIF: Chief terminated with error \n");  
            return err;
        }

        strncpy(ready, convey[Ingredients_num - 1], Product_len);

        err = sem_p(sem_id, Ready_sem);
        if (err != 0)
        {
            fprintf(stderr, "CHIF: Chief terminated with error \n");
            return err;
        }

        for (int iter = Ingredients_num - 1; iter > 0; iter--)
        {
            strncpy(convey[iter], convey[iter - 1], Product_len);
        }

        strncpy(convey[0], empty, Product_len);

        fprintf(stderr, "CHIF: moving finished \n");

        fprintf(stderr, "CHIF: Empty    |%s| \n", empty);
        fprintf(stderr, "CHIF: P_convey |%s| \n", convey[0]);
        fprintf(stderr, "CHIF: I_convey |%s| \n", convey[1]);
        fprintf(stderr, "CHIF: Z_convey |%s| \n", convey[2]);
        fprintf(stderr, "CHIF: A_convey |%s| \n", convey[3]);
        fprintf(stderr, "CHIF: Ready    |%s| \n", ready);

        for (unsigned iter = 0; iter < Ingredients_num; iter++)
        {
            *(wake_up_next_arr[iter]) = true;
        }

        if (strlen(ready) != 0)
        {
            fprintf(stderr, "CHIF: Chief tells client that pizza is ready \n");

            err = sem_v(sem_id, Client_sem);
            if (err != 0)
            {
                fprintf(stderr, "CHIF: Chief terminated with error \n");
                return err;
            }
        }

        struct sembuf ops_free[] = {{.sem_num = P_convey_sem, .sem_op = 1, .sem_flg = 0},
                                    {.sem_num = Ready_sem,    .sem_op = 1, .sem_flg = 0}};
                                    
        err = semop(sem_id, ops_free, sizeof(ops_free) / sizeof(struct sembuf));
        if (err != 0)
        {
            ERR(fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno)));
            fprintf(stderr, "CHIF: Chief terminated with error \n");
            return -errno;
        }    
    }

    return 0;
}

//---------------------------------------------------------