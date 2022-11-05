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

struct sembuf Sem_v = {.sem_num = 0, .sem_op = +1, .sem_flg = 0};

struct sembuf Sem_p = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};

//=========================================================

static const unsigned Process_num = 20;
static const unsigned Incr_num    = 500000;

//=========================================================

static int child(int sem_id, int* value, unsigned incr_num);

//=========================================================

static int child(int sem_id, int* value, unsigned incr_num)
{
    assert(value);

    int local_val = 0;

    for (unsigned j = 0; j < incr_num; j++)
    {
        local_val += 1;
    }

    struct sembuf op = Sem_p;
    op.sem_flg |= SEM_UNDO;

    int err = semop(sem_id, &op, 1);
    if (err != 0)
    {
        fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    *value += local_val;

    // There is no point of detaching shared memory in child process

    return 0;
}

//---------------------------------------------------------

int main()
{
    int shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0700);
    if (shm_id == -1)
    {
        fprintf(stderr, "shmget() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    int* value = shmat(shm_id, NULL, 0);
    if (value == (void*) (-1))
    {
        fprintf(stderr, "shmat() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    int sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0700);
    if (sem_id == -1)
    {
        fprintf(stderr, "semget() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    int err = semop(sem_id, &Sem_v, 1);
    if (err != 0)
    {
        fprintf(stderr, "semop() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    *value = 0;
    pid_t pid = 0;

    struct timeval start_time = {};
    int get_time_err = gettimeofday(&start_time, NULL);
    if (get_time_err == -1)
    {
        fprintf(stderr, "gettimeofday() syscall failed: %s\n", strerror(errno));
        return -1;
    }

    for (unsigned iter = 0; iter < Process_num; iter++)
    {
        pid = fork();
        if (pid == 0)
        {
            return child(sem_id, value, Incr_num);
        }
        else
            continue;
    }

    for (unsigned iter = 0; iter < Process_num; iter++)
    {
        pid_t term = wait(NULL);
        if (term == -1)
        {
            fprintf(stderr, "wait() syscall failed: %s \n", strerror(errno));
            return -errno;
        }
    }

    struct timeval end_time = {};
    get_time_err = gettimeofday(&end_time, NULL);
    if (get_time_err == -1)
    {
        fprintf(stderr, "gettimeofday() syscall failed: %s\n", strerror(errno));
        return -1;
    }

    struct timeval dif = {};
    timersub(&end_time, &start_time, &dif);

    putchar('\n');

    printf("Start moment: %s\n", ctime(&start_time.tv_sec));
    printf("End   moment: %s\n", ctime(&end_time  .tv_sec));

    printf("Elapsed time: %ld seconds "
                            "%ld milliseconds "
                            "%ld microsecond \n", dif.tv_sec, dif.tv_usec / 1000, 
                                                              dif.tv_usec % 1000);

    putchar('\n');
    printf("Total value: %d \n", *value);

    err = semctl(sem_id, 1, IPC_RMID);
    if (err == -1)
    {
        fprintf(stderr, "semctl() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    err = shmdt(value);
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
}