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

//=========================================================

struct Cook_convey_sem
{
    int sem_id;

    unsigned sem_ingredient_access;
    unsigned sem_ingredient_num;

    unsigned sem_convey;
};

//=========================================================

static int check_terminate(int sem_id);

static int sem_v(int sem_id, unsigned sem_num);

static int sem_p(int sem_id, unsigned sem_num);

static int sem_oper(int sem_id, unsigned short sem_num, short sem_op, short sem_flg);

static int cook_ingredients(int sem_id, char* ingredients, unsigned* ingredient_sems_access,
                                                           unsigned* ingredient_sems_num, char** storage);

static int client          (int sem_id, char* ready);

static int cook_convey     (const struct Cook_convey_sem* cook_convey_sem, 
                            char ingredient, unsigned count, char* storage, char* workplace);

static int chief           (int sem_id, char** convey, char* ready, char* empty);

//=========================================================

static const unsigned Pizza_num = 10;

static const unsigned Ingredient_low  = 5;
static const unsigned Ingredient_high = 10;

static const unsigned Ingredients_num = 4;

static const unsigned Product_len = strlen("pizza");

//---------------------------------------------------------

static const unsigned Sem_num = 14;

static const unsigned P_ingredient_access = 0;
static const unsigned I_ingredient_access = 1;
static const unsigned Z_ingredient_access = 2;
static const unsigned A_ingredient_access = 3;

static const unsigned P_ingredient_num = 4;
static const unsigned I_ingredient_num = 5;
static const unsigned Z_ingredient_num = 6;
static const unsigned A_ingredient_num = 7;

static const unsigned P_convey_sem = 8;
static const unsigned I_convey_sem = 9;
static const unsigned Z_convey_sem = 10;
static const unsigned A_convey_sem = 11;

static const unsigned Ready_sem    = 12;

static const unsigned Terminate_sem = 13;

//=========================================================

int main()
{
    // prepare all for work

    size_t size = sizeof(char) * (4 * Ingredient_high + 6 * strlen("pizza"));

    int shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | 0700);
    void* addr = shmat(shm_id, NULL, 0);

    memset(addr, 0, size);

    char* empty = (char*) addr;
    char* ready = (char*) (addr + Product_len * 5);

    char** convey = {(char*) (addr + Product_len),
                     (char*) (addr + Product_len * 2),
                     (char*) (addr + Product_len * 3),
                     (char*) (addr + Product_len * 4)};

    char     ingredients      [Ingredients_num] = {'p'  , 'i'  , 'z'  , 'a'};
    unsigned ingredients_count[Ingredients_num] = {1, 1, 2, 1};

    unsigned ingredient_sems_access[Ingredients_num] = {P_ingredient_access, 
                                                       I_ingredient_access, 
                                                       Z_ingredient_access, 
                                                       A_ingredient_access};

    unsigned ingredient_sems_num[Ingredients_num] = {P_ingredient_num, 
                                                     I_ingredient_num, 
                                                     Z_ingredient_num, 
                                                     A_ingredient_num};


    unsigned convey_sem_nums[Ingredients_num] = {P_convey_sem, 
                                                 I_convey_sem, 
                                                 Z_convey_sem, 
                                                 A_convey_sem};

    char** cook_storage = {(char*) (addr + Product_len * 6),
                           (char*) (addr + Product_len * 6 + Ingredient_high),
                           (char*) (addr + Product_len * 6 + Ingredient_high * 2),
                           (char*) (addr + Product_len * 6 + Ingredient_high * 3)};

    int sem_id = semget(IPC_PRIVATE, Sem_num, IPC_CREAT | IPC_EXCL | 0700);
    if (sem_id == -1) 
    {
        fprintf(stderr, "semget() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    pid_t pid = 0;

    // set semaphores to their initial values

    struct sembuf initial[9] = { {.sem_num = P_ingredient_access, .sem_op = +1, .sem_flg = 0},
                                 {.sem_num = I_ingredient_access, .sem_op = +1, .sem_flg = 0},
                                 {.sem_num = Z_ingredient_access, .sem_op = +1, .sem_flg = 0},
                                 {.sem_num = A_ingredient_access, .sem_op = +1, .sem_flg = 0},
                                 {.sem_num = P_convey_sem,        .sem_op = +1, .sem_flg = 0},
                                 {.sem_num = I_convey_sem,        .sem_op = +1, .sem_flg = 0},
                                 {.sem_num = Z_convey_sem,        .sem_op = +1, .sem_flg = 0},
                                 {.sem_num = A_convey_sem,        .sem_op = +1, .sem_flg = 0}};

    int err = semop(sem_id, initial, 9);
    if (err != 0)
    {
        fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    // run child processes

    for (unsigned iter = 0; iter < Ingredients_num; iter++)
    {
        pid = fork();

        if (pid == 0)
        {
            struct Cook_convey_sem cur = {.sem_id = sem_id, .sem_ingredient_access = ingredient_sems_access[iter],
                                                            .sem_ingredient_num    = ingredient_sems_num   [iter], 
                                                            .sem_convey            = convey_sem_nums       [iter]};

            return cook_convey(&cur, ingredients[iter], 
                                     ingredients_count[iter], cook_storage[iter], convey[iter]);
        }
        else if (pid == -1)
        {
            fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno));
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
        fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    pid = fork();
    if (pid == 0)
    {
        return chief(sem_id, convey, ready, empty);
    }
    else if (pid == -1)
    {
        fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    pid = fork();
    if (pid == 0)
    {
        return client(sem_id, ready);
    }
    else if (pid == -1)
    {
        fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    // wait all child processes to terminate

    for (unsigned iter = 0; iter < Ingredients_num + 3; iter++)
    {   
        pid_t term = wait(NULL);
        if (term == -1)
        {
            fprintf(stderr, "wait() syscall failed: %s \n", strerror(errno));
            return -errno;
        }
    }

    // deattach and/or delete semaphores and shared memory

    err = semctl(sem_id, 1, IPC_RMID);
    if (err == -1)
    {
        fprintf(stderr, "semctl() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    err = shmdt(addr);
    if (err == -1)
    {
        fprintf(stderr, "shmdt() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    err = shmctl(shm_id, IPC_RMID, NULL);
    if (err == -1)
    {
        fprintf(stderr, "shmctl() syscall failed: %s \n", strerror(errno));
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
        fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno));
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
        fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno));
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
        fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno));
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
        fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno));
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

    while (1)
    {
        int term = 0;
        if ((term = check_terminate(sem_id)) == 1)
        {
            return 0;
        }
        else if (term != 0)
        {
            return term;
        }

        for (unsigned iter = 0; iter < Ingredients_num; iter++)
        {
            int err = sem_p(sem_id, ingredient_sems_access[iter]);
            if (err != 0)
                return err;

            int ingredient_amount = strlen(storage[iter]);

            if (ingredient_amount <= Ingredient_low)
            {
                int cooked = 0;
                char ingredient = ingredients[iter];

                while(ingredient_amount <= Ingredient_high)
                {
                    strcat(storage[iter], &ingredient);
                    cooked++;
                }

                err = sem_oper(sem_id, ingredient_sems_num[iter], cooked, 0);
                if (err != 0)
                    return err;
            }

            err = sem_v(sem_id, ingredient_sems_num[iter]);
            if (err != 0)
                return err;
        }
        
    }

    return 0;
}

//---------------------------------------------------------

static int client(int sem_id, char* ready)
{
    assert(ready);

    unsigned pizza_count = 0;

    while (pizza_count < Pizza_num)
    {
        int err = sem_p(sem_id, Ready_sem);
        if (err != 0)
            return err;

        if (strcmp(ready, "pizza") == 0)
        {
            pizza_count++;
            *ready = '\0';
        }
    }

    return sem_oper(sem_id, Terminate_sem, Ingredients_num + 2, 0);
}

//---------------------------------------------------------

static int cook_convey(const struct Cook_convey_sem* cook_convey_sem, 
                       char ingredient, unsigned count, char* storage, char* workplace)
{
    assert(cook_convey_sem);
    assert(storage);
    assert(workplace);
}

//---------------------------------------------------------

static int chief(int sem_id, char** convey, char* ready, char* empty)
{
    assert(convey);
    assert(ready);
    assert(empty);

    while (1)
    {
        int term = 0;
        if ((term = check_terminate(sem_id)) == 1)
        {
            return 0;
        }
        else if (term != 0)
        {
            return term;
        }

        if (strlen(ready) == Product_len) // fix
        {
            struct sembuf ops[Ingredients_num] = {{.sem_num = P_convey_sem, .sem_op = -1, .sem_flg = 0},
                                                  {.sem_num = I_convey_sem, .sem_op = -1, .sem_flg = 0},
                                                  {.sem_num = Z_convey_sem, .sem_op = -1, .sem_flg = 0},
                                                  {.sem_num = A_convey_sem, .sem_op = -1, .sem_flg = 0}};
            int err = semop(sem_id, ops, Ingredients_num);
            if (err != 0)
            {
                fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno));
                return -errno;
            }

            strncpy(ready, convey[Ingredients_num - 1], Product_len);

            for (int iter = Ingredients_num - 1; iter > 0; iter--)
            {
                strncpy(convey[iter], convey[iter - 1], Product_len);
            }

            strncpy(convey[0], empty, Product_len);

            for (unsigned iter = 0; iter < Ingredients_num; iter++)
                ops[iter].sem_op = +1;

            err = semop(sem_id, ops, Ingredients_num);
            if (err != 0)
            {
                fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno));
                return -errno;
            }

            err = sem_v(sem_id, Ready_sem);
            if (err != 0)
                return err;
        }
    }
}

//---------------------------------------------------------