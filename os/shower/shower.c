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
#include <signal.h>

//=========================================================

#define DEBUG

#ifdef DEBUG

    #define DBG(...)                                        \
                                                            \
    do                                                      \
    {                                                       \
    __VA_ARGS__;                                            \
    } while(0);

#else 

    #define DBG(...) 

#endif 

//=========================================================

#define ERR_LOC(action)                                     \
                                                            \
    do                                                      \
    {                                                       \
                                                            \
        fprintf(stderr, "File: %s ",  __FILE__);            \
        fprintf(stderr, "Line: %d\n", __LINE__);            \
        action;                                             \
                                                            \
    } while (0);                                

//---------------------------------------------------------

#define ERROR(failed_func) ERR_LOC(fprintf(stderr, #failed_func " failed: %s \n", strerror(errno)));

//---------------------------------------------------------

#define ERROR_RET(failed_func, ret_val)                     \
                                                            \
    do                                                      \
    {                                                       \
                                                            \
        ERROR(failed_func);                                 \
        return ret_val;                                     \
                                                            \
    } while (0);
    

//=========================================================

static const unsigned Sem_num    = 5;

static const unsigned Shower_sem = 0;
static const unsigned Men        = 1;
static const unsigned Women      = 2;

static const unsigned M_schedule_sem = 3;
static const unsigned W_schedule_sem = 4;

//=========================================================

enum Sex
{
    WOMAN = 0,
    MAN   = 1 
};

//---------------------------------------------------------

static int consumer(const int sem_id, enum Sex sex, const unsigned num);

static int scheduler(const int sem_id);

//=========================================================

int main(const int argc, const char** argv)
{
    int err = 0;

    unsigned w_num = atoi(argv[1]);
    unsigned m_num = atoi(argv[2]);

    unsigned cap = atoi(argv[3]);
    
    //-----------------------

    int sem_id = semget(IPC_PRIVATE, Sem_num, IPC_CREAT | IPC_EXCL | 0700);
    if (sem_id == -1) ERROR_RET(semget, -errno);

    struct sembuf initial[] = {{.sem_num = Shower_sem,     .sem_flg = 0, .sem_op = cap},
                               {.sem_num = W_schedule_sem, .sem_flg = 0, .sem_op = +1}};
    err = semop(sem_id, initial, 2);
    if (err != 0) ERROR_RET(semop, -errno);

    //-----------------------

    pid_t pid = 0;
    unsigned total_ct = 0;

    pid_t scheduler_pid = fork();
    if (scheduler_pid == 0)
    {
        return scheduler(sem_id);
    }
    else if (pid == -1) ERROR_RET(fork, -errno);

    for (unsigned iter = 0; iter < w_num; iter++)
    {
        pid = fork();

        if (pid == 0)
        {
            return consumer(sem_id, WOMAN, total_ct);
        }
        else if (pid == -1) ERROR_RET(fork, -errno);

        total_ct++;
    }

    for (unsigned iter = 0; iter < m_num; iter++)
    {
        pid = fork();

        if (pid == 0)
        {
            return consumer(sem_id, MAN, total_ct);
        }
        else if (pid == -1) ERROR_RET(fork, -errno);

        total_ct++;
    }

    //-----------------------

    unsigned consumers_total = w_num + m_num;

    for (unsigned iter = 0; iter < consumers_total; iter++)
    {
        pid_t term = wait(NULL);
        if (term == -1) ERROR_RET(wait, -errno);
    }

    err = kill(scheduler_pid, SIGTERM);
    if (err == -1) ERROR_RET(kill, -errno);

    //-----------------------

    err = semctl(sem_id, Sem_num, IPC_RMID);
    if (err == -1) ERROR_RET(semctl, -errno);

    return 0;
}

//---------------------------------------------------------

static int scheduler(const int sem_id)
{
    int err = 0;

    DBG(fprintf(stderr, "\n SHD started \n"));

    bool men = true;

    while (1)
    {
        if (men)
        {
            struct sembuf op[] = {{.sem_num = M_schedule_sem, .sem_op = +1, .sem_flg = 0},
                                  {.sem_num = W_schedule_sem, .sem_op = -1, .sem_flg = 0}};

            err = semop(sem_id, op, 2);
            if (err == -1) ERROR_RET(semop, -errno);

            DBG(fprintf(stderr, "SHD switched timetable from M to W \n"));
            men = false;
        }
        else 
        {
            struct sembuf op[] = {{.sem_num = M_schedule_sem, .sem_op = -1, .sem_flg = 0},
                                  {.sem_num = W_schedule_sem, .sem_op = +1, .sem_flg = 0}};

            err = semop(sem_id, op, 2);
            if (err == -1) ERROR_RET(semop, -errno);

            DBG(fprintf(stderr, "SHD switched timetable from W to M \n"));
            men = true;
        }

        usleep(100);
    }

    return 0;
}

//---------------------------------------------------------

static int consumer(const int sem_id, enum Sex sex, const unsigned num)
{
    int err = 0;

    DBG(fprintf(stderr, "CSM#%d SEX:%d started \n", num, (int) sex));

    DBG(fprintf(stderr, "CSM#%d SEX:%d about to get into shower \n", num, (int) sex));
    
    struct sembuf entry[4] = { 0 };

    entry[0].sem_num = Shower_sem;
    entry[0].sem_op  = -1;
    entry[0].sem_flg = 0;

    entry[1].sem_num = (sex == MAN)? Men : Women;
    entry[1].sem_op  = +1;
    entry[1].sem_flg = 0;

    entry[2].sem_num = (sex == MAN)? Women : Men;
    entry[2].sem_op  = 0;
    entry[2].sem_flg = 0;

    entry[3].sem_num = (sex == MAN)? M_schedule_sem : W_schedule_sem;
    entry[3].sem_op  = 0;
    entry[3].sem_flg = 0;

    err = semop(sem_id, entry, 4);
    if (err != 0) ERROR_RET(semop, -errno);

    DBG(fprintf(stderr, "CSM#%d SEX:%d showering \n", num, (int) sex));

    usleep(50);

    DBG(fprintf(stderr, "CSM#%d SEX:%d stopped showering \n", num, (int) sex));

    struct sembuf exit[2] = { 0 };

    exit[0].sem_num = Shower_sem;
    exit[0].sem_op  = +1;
    exit[0].sem_flg = 0;

    exit[1].sem_num = (sex == MAN)? Men : Women;
    exit[1].sem_op  = -1;
    exit[1].sem_flg = 0;

    err = semop(sem_id, exit, 2);
    if (err != 0) ERROR_RET(semop, -errno);

    DBG(fprintf(stderr, "CSM#%d SEX:%d terminated \n", num, (int) sex));
    return 0;
}