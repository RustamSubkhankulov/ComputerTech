#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <time.h>

//---------------------------------------------------------

#include "stad.h"

//=========================================================

static int get_runners_num(const int argc, const char** argv);

static int judge_wait_arrive(const int msg_q_id, const unsigned runners_num);

static int jugde (const int msg_q_id, const unsigned runners_num);

static int runner(const int msg_q_id, const unsigned number, const int runners_num);

//=========================================================

// runners are numered from 1 to N

enum Message_val
{
    ARRIVED = 0,
    START   = 1,
    FINISH  = 2
};

struct Message
{
    long msg_type;
    Message_val val;
};

//=========================================================

int execute_stad(const int argc, const char** argv)
{
    int num = get_runners_num(argc, argv);
    if (num < 0)
        return num;

    unsigned runners_num = (unsigned) num;

    int msg_q_id = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0700);
    if (msg_q_id == -1)
    {
        fprintf(stderr, "msgget() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    int pid = fork();
    if (pid == 0)
    {   
        return jugde(msg_q_id, runners_num);
    }
    else if (pid == -1)
    {
        fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    for (unsigned iter = 0; iter < runners_num; iter++)
    {
        pid = fork();
        if (pid == 0)
        {
            return runner(msg_q_id, iter + 1, runners_num);
        }
        else if (pid == -1)
        {
            fprintf(stderr, "fork() syscall failed: %s \n", strerror(errno));
            return -errno;
        }
    }

    for (unsigned iter = 0; iter < runners_num + 1; iter++)
    {
        pid_t term = wait(NULL);
        if (term == -1)
        {
            fprintf(stderr, "wait() syscall failed: %s \n", strerror(errno));
            return -errno;
        }
    }

    int err = msgctl(msg_q_id, IPC_RMID, NULL);
    if (err != 0)
    {
        fprintf(stderr, "msgctl() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    return 0;
}

//---------------------------------------------------------

static int get_runners_num(const int argc, const char** argv)
{
    assert(argv);

    if (argc != 2)
        return INC_ARGS;

    return atoi(argv[1]);
}

//---------------------------------------------------------

static int jugde(const int msg_q_id, const unsigned runners_num)
{
    printf("Judge is waiting for all of the runners at stadium \n");

    int err = judge_wait_arrive(msg_q_id, runners_num);
    if (err)
        return err;

    printf("All runners arrived. Judge is starting to measure time. \n");
    
    struct timeval start_time = {};
    int get_time_err = gettimeofday(&start_time, NULL);
    if (get_time_err == -1)
    {
        fprintf(stderr, "gettimeofday() syscall failed: %s\n", strerror(errno));
        return -1;
    }

    printf("Competition starts mow! \n");

    struct Message msg = {.msg_type = 1, .val = START};

    err = msgsnd(msg_q_id, &msg, sizeof(Message_val), 0);
    if (err == -1)
    {
        fprintf(stderr, "msgsnd() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    printf("Judge has already sent msg to 1st runner \n");
    
    while(1)
    {
        int size = msgrcv(msg_q_id, &msg, sizeof(Message_val), runners_num + 1, 0);
        if (size == -1)
        {
            fprintf(stderr, "msgrcv() syscall failed: %s \n", strerror(errno));
            return -errno;
        }

        if (msg.val == START)
            break;
    }

    printf("Judge has already received msg from last runner \n");
    printf("Competition ends now. Judge stops measuring time. \n");

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

    return 0;
}

//---------------------------------------------------------

static int judge_wait_arrive(const int msg_q_id, const unsigned runners_num)
{
    struct Message msg = { 0 };

    int msg_num = 0;

    while(msg_num < runners_num)
    {
        int size = msgrcv(msg_q_id, &msg, sizeof(Message_val), runners_num + 1, 0);
        if (size == -1)
        {
            fprintf(stderr, "msgrcv() syscall failed: %s \n", strerror(errno));
            return -errno;
        }

        if (msg.val == ARRIVED)
            msg_num++;
        else
            continue;
    }

    return 0;
}

//---------------------------------------------------------

static int runner(const int msg_q_id, const unsigned number, const int runners_num)
{
    struct Message msg = { .msg_type = runners_num + 1, .val = ARRIVED}; // for judge 

    printf("Runner %d has just arrived \n", number);

    int err = msgsnd(msg_q_id, &msg, sizeof(Message_val), 0);
    if (err == -1)
    {
        fprintf(stderr, "msgsnd() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    while(1)
    {
        int size = msgrcv(msg_q_id, &msg, sizeof(Message_val), number, 0);
        if (size == -1)
        {
            fprintf(stderr, "msgrcv() syscall failed: %s \n", strerror(errno));
            return -errno;
        }

        if (msg.val == START)
            break;
    }

    printf("Runner %d received msg and started \n", number);

    msg.msg_type = number + 1;
    msg.val      = START;

    printf("Runner finished \n");

    err = msgsnd(msg_q_id, &msg, sizeof(Message_val), 0);
    if (err == -1)
    {
        fprintf(stderr, "msgsnd() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    if (number == runners_num)
        printf("Runner %d sent msg to judge \n", number);
    else
        printf("Runner %d passed the baton \n", number);

    return 0;
}