#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>              
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <poll.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <sys/wait.h>

//=========================================================

// #define DEBUG

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

#define INIT_POLLFD(pollfds, ind, fdescr, evnt, revnt)      \
                                                            \
    do                                                      \
    {                                                       \
        pollfds[ind].fd = fdescr;                           \
        pollfds[ind].events  = evnt;                        \
        pollfds[ind].revents = revnt;                       \
                                                            \
    } while (0);                                    
    

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

struct Child_pipe
{
    pid_t pid;

    int fdin;
    int fdout;
};

//---------------------------------------------------------

static const size_t Buffer_size = 4096ul;

struct Buffer
{
    char* buffer;
    size_t cnt;
    size_t ofs;
};

//---------------------------------------------------------

// static const struct timespec Inf_timeout = {.tv_sec = -1, .tv_nsec = 0};
static const int Inf_timeout = -1;

//=========================================================

static int execute_check(struct Child_pipe* child_pipes, unsigned prog_times);

static struct Buffer* alloc_buffers(unsigned prog_times);

static void free_buffers(struct Buffer* buffers, unsigned prog_times);

static int perform_io(struct Child_pipe* child_pipes, unsigned prog_times, 
                                   struct pollfd* pollfds, struct Buffer* buffers);

static void setup_start_pollfds(struct Child_pipe* child_pipes, unsigned prog_times, 
                                                            struct pollfd* pollfds);

static int handle_write_fd(unsigned iter, struct Buffer* buffers, 
                            struct pollfd* pollfds, uint64_t* out_checksum);

static int handle_read_fd (unsigned iter, struct Buffer* buffers, 
                            struct pollfd* pollfds, uint64_t* in_checksum, int* cur_pollfdn);

//=========================================================

int main(const int argc, const char** argv)
{
    assert(argc == 3);

    const char* prog_name = argv[1];

    errno = 0;
    unsigned prog_times = (unsigned) atoi(argv[2]);

    DBG(fprintf(stderr, "prog_times == %d \n", prog_times));

    // Array describing children processes and their pipes
    struct Child_pipe* child_pipes = (struct Child_pipe*) calloc(prog_times, sizeof(struct Child_pipe));
    assert(child_pipes);

    int pid = 0;
    int pipe_fd_in [2] = { 0 };
    int pipe_fd_out[2] = { 0 };

    for (unsigned iter = 0; iter < prog_times; iter++)
    {
        int err = pipe(pipe_fd_in);
        if (err == -1)
        {
            ERR(fprintf(stderr, "pipe() failed: %s \n", strerror(errno)));
            return -errno;
        }

        err = pipe(pipe_fd_out);
        if (err == -1)
        {
            ERR(fprintf(stderr, "pipe() failed: %s \n", strerror(errno)));
            return -errno;
        }

        pid = fork();
        if (err == -1)
        {
            ERR(fprintf(stderr, "fork() failed: %s \n", strerror(errno)));
            return -errno;
        }

        if (pid == 0) // child
        {
            int input  = pipe_fd_in[0]; 
            int output = pipe_fd_out[1];

            err = dup2(input, 0);
            if (err == -1)
            {
                ERR(fprintf(stderr, "dup2() failed: %s \n", strerror(errno)));
                return -errno;
            }

            err = dup2(output, 1);
            if (err == -1)
            {
                ERR(fprintf(stderr, "dup2() failed: %s \n", strerror(errno)));
                return -errno;
            }

            // close pipes as we already reopened them using dup2
            for (unsigned ind = 0; ind < 2; ind++)
            {
                err = close(pipe_fd_in[ind]);
                if (err == -1)
                {
                    ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                    return -errno;
                }

                err = close(pipe_fd_out[ind]);
                if (err == -1)
                {
                    ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                    return -errno;
                }
            }

            // close all fd's inherited from parent
            for (unsigned ind = 0; ind < iter; ind++)
            {
                err = close(child_pipes[ind].fdin);
                if (err == -1)
                {
                    ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                    return -errno;
                }

                err = close(child_pipes[ind].fdout);
                if (err == -1)
                {
                    ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                    return -errno;
                }
            }

            // executing testing prog

            char* const prog_argv[] = {(char* const) prog_name, NULL};

            err = execvp(prog_name, prog_argv);
            if (err == -1)
            {
                ERR(fprintf(stderr, "execvp() failed: %s \n", strerror(errno)));
                return -errno;
            }

            return 0;
        }
        else if (pid != 0) // parent
        {
            // remember needed fd's
            child_pipes[iter].fdin  = pipe_fd_in [1];
            child_pipes[iter].fdout = pipe_fd_out[0];

            // close unessessary fd's
            err = close(pipe_fd_in[0]);
            if (err == -1)
            {
                ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                return -errno;
            }

            err = close(pipe_fd_out[1]);
            if (err == -1)
            {
                ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                return -errno;
            }
        }
    }

    int err = execute_check(child_pipes, prog_times);
    if (err != 0) fprintf(stderr, "execute_check() failed \n");

    // wait every child to terminate
    for (unsigned iter = 0; iter < prog_times; iter++)
    {
        err = wait(NULL);
        if (err == -1)
        {
            ERR(fprintf(stderr, "wait() failed: %s \n", strerror(errno)));
            return -errno;
        }
    }

    free(child_pipes);

    return 0;
}

//---------------------------------------------------------

static struct Buffer* alloc_buffers(unsigned prog_times)
{
    struct Buffer* buffers = (struct Buffer*) calloc(prog_times + 1, sizeof(struct Buffer));
    assert(buffers);

    for (unsigned iter = 0; iter < prog_times + 1; iter++)
    {
        buffers[iter].buffer = (char*) calloc(Buffer_size, sizeof(char));
        assert(buffers[iter].buffer);

        buffers[iter].cnt = 0;
        buffers[iter].ofs = 0;
    }

    return buffers;
}

//---------------------------------------------------------

static void free_buffers(struct Buffer* buffers, unsigned prog_times)
{
    assert(buffers);

    for (unsigned iter = 0; iter < prog_times + 1; iter++)
    {
        assert(buffers[iter].cnt == 0);
        free(buffers[iter].buffer);
    }

    free(buffers);
}

//---------------------------------------------------------

static int perform_io(struct Child_pipe* child_pipes, unsigned prog_times, 
                                   struct pollfd* pollfds, struct Buffer* buffers)
{
    assert(child_pipes);
    assert(pollfds);
    assert(buffers);

    uint64_t  in_checksum = 0;
    uint64_t out_checksum = 0;

    const nfds_t pollfdn = prog_times * 2 + 2;
    int pollfdn_active = (int) pollfdn;

    while(1)
    {
        int ready = poll(pollfds, pollfdn, -1);
        if (ready == -1)
        {
            ERR(fprintf(stderr, "poll() failed: %s \n", strerror(errno)));
            return -errno;
        }

        DBG(fprintf(stderr, "//-------------------------------------- \n"));
        DBG(fprintf(stderr, "ready is %d \n", ready));

        unsigned iter = 0;

        for (unsigned ready_ct = 0; ready_ct < ready; ready_ct++)
        {
            while (pollfds[iter].revents == 0)
            {
                iter++;
                assert(iter < pollfdn);
            }

            DBG(fprintf(stderr, "NOW WORKING WITH iter == %d \n", iter));

            if (pollfds[iter].revents & (POLLERR | POLLNVAL))
            {
                ERR(fprintf(stderr, "poll(): unexpected revents filed value: %x \n", pollfds[iter].revents));
                return -1;
            }

            if ((iter % 2) == 0) // even fd's - read
            {
                int err = handle_read_fd(iter, buffers, pollfds, &in_checksum, &pollfdn_active);
                if (err != 0) 
                {
                    ERR(fprintf(stderr, "handle_read_fd() failed \n"));
                    return err;
                }
            }   
            else                // odd fd's - write
            {
                int err = handle_write_fd(iter, buffers, pollfds, &out_checksum);
                if (err != 0) 
                {
                    ERR(fprintf(stderr, "handle_write_fd() failed \n"));
                    return err;
                }
            }        
        
            DBG(fprintf(stderr, "pollfdn_active == %d \n", pollfdn_active));
        }

        if (pollfdn_active == 2)
                break;
    }

    DBG(fprintf(stderr, "IO is stopped \n"));

    return (in_checksum == out_checksum);
}

//---------------------------------------------------------

static int handle_write_fd(unsigned iter, struct Buffer* buffers, 
                           struct pollfd* pollfds, uint64_t* out_checksum)
{
    assert(buffers);
    assert(pollfds);
    assert(out_checksum);

    unsigned buffer_ind = iter / 2;
    DBG(fprintf(stderr, "buffer_ind is %u \n", buffer_ind));

    DBG(fprintf(stderr, "write pollfd: iter == %d \n", iter));

    if (buffers[buffer_ind].cnt != 0) // buffer is not empty, write is possible
    {
        DBG(fprintf(stderr, "offs is now %ld \n", buffers[buffer_ind].ofs));

        ssize_t write_ct = write(pollfds[iter].fd, buffers[buffer_ind].buffer, 
                                                   buffers[buffer_ind].cnt + buffers[buffer_ind].ofs);
        if (write_ct == -1)
        {
            ERR(fprintf(stderr, "write() failed: %s \n", strerror(errno)));
            return -errno;
        }

        DBG(fprintf(stderr, "written %ld \n", write_ct));

        buffers[buffer_ind].cnt -= write_ct;

        if (write_ct != 0 && pollfds[iter].fd != 1)
        {
            pollfds[iter + 1].events = POLLIN;

            DBG(fprintf(stderr, "written nonzero data amount, set pollfds[%d].events to %d \n", iter + 1, 
                                                                                        pollfds[iter + 1].events));
        }

        if (pollfds[iter].fd == 1)
        {
            DBG(fprintf(stderr, "recalculating out checksum: prev == %lu \n", *out_checksum));

            for (unsigned ind = 0; ind < write_ct; ind++)
                (*out_checksum) += buffers[buffer_ind].buffer[buffers[buffer_ind].ofs + ind];

            DBG(fprintf(stderr, "recalculated out checksum: new == %lu \n", *out_checksum));
        }

        if (buffers[buffer_ind].cnt == 0) // buffer is now empty
        {
            DBG(fprintf(stderr, "buffer is now empty \n"));

            buffers[buffer_ind].ofs = 0;

            pollfds[iter].events = 0;

            if (pollfds[iter - 1].fd == 0)
                pollfds[iter - 1].events = POLLIN;

            DBG(fprintf(stderr, "pollfds[%d].events == %d "
                            "and pollfds[%d].events == %d \n", iter,     pollfds[iter].events,
                                                               iter - 1, pollfds[iter - 1].events));
        }
        else // there are still 
        {
            DBG(fprintf(stderr, "buffer is not empty after write \n"));

            buffers[buffer_ind].ofs + write_ct;

            DBG(fprintf(stderr, "ofs after write is %ld \n", buffers[buffer_ind].ofs));
        }
    }
    else // buffer is empty, write is impossible
    {
        DBG(fprintf(stderr, "buffer is empty, write is impossible \n"));

        pollfds[iter].events = 0;
    }

    pollfds[iter].revents = 0;

    return 0;
}

//---------------------------------------------------------

static int handle_read_fd(unsigned iter, struct Buffer* buffers, 
                          struct pollfd* pollfds, uint64_t* in_checksum, int* cur_pollfdn)
{
    assert(buffers);
    assert(pollfds);

    unsigned buffer_ind = iter / 2;        
    DBG(fprintf(stderr, "buffer_ind is %u \n", buffer_ind));

    DBG(fprintf(stderr, "read pollfd: iter == %d fd == %d revents == %#x \n", 
                                                        iter, pollfds[iter].fd, 
                                                              pollfds[iter].revents));

    if (pollfds[iter].revents & POLLHUP) // other end of pipe is close, closing this end
    {
        DBG(fprintf(stderr, "POLLHUP on iter == %d \n", iter));

        if (pollfds[iter].fd != 0) // close if it is not stdin
        {
            int err = close(pollfds[iter].fd);
            if (err != 0)
            {
                ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                return -errno;
            }

            (*cur_pollfdn) -= 1;
        }

        if (pollfds[iter + 1].fd != 1) // close if it is not stdout
        {
            int err = close(pollfds[iter + 1].fd);
            if (err != 0)
            {
                ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                return -errno;
            }

            (*cur_pollfdn) -= 1;
        }

        DBG(fprintf(stderr, "closed %d and %d, pollfd_active == %d \n", 
                            pollfds[iter].fd, pollfds[iter + 1].fd, *cur_pollfdn));

        pollfds[iter].    fd = 0;
        pollfds[iter + 1].fd = 0;

        pollfds[iter].    events = 0;
        pollfds[iter + 1].events = 0;

        pollfds[iter].revents = 0;
        return 0;
    }
    else 
        DBG(fprintf(stderr, "revents == %#x \n", pollfds[iter].revents));

    if (buffers[buffer_ind].cnt == 0) // buffer is free, read is possible
    {
        DBG(fprintf(stderr, "buffer is free \n"));

        ssize_t read_ct = read(pollfds[iter].fd, buffers[buffer_ind].buffer, Buffer_size);
        if (read_ct == -1)
        {
            ERR(fprintf(stderr, "read() failed: %s \n", strerror(errno)));
            return -errno;
        }

        DBG(fprintf(stderr, "read %ld \n", read_ct));

        if (pollfds[iter].fd == 0) // checksum
        {
            DBG(fprintf(stderr, "recalculating checksum: prev == %lu \n", *in_checksum));

            for (unsigned ind = 0; ind < read_ct; ind++)
                (*in_checksum) += buffers[buffer_ind].buffer[ind];
        
            DBG(fprintf(stderr, "recalculated checksum: new == %lu \n", *in_checksum));
        }

        if (read_ct == 0) // EOF
        {
            DBG(fprintf(stderr, "EOF is reached \n"));

            if (pollfds[iter].fd != 0) // close if it is not stdin
            {
                int err = close(pollfds[iter].fd);
                if (err != 0)
                {
                    ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                    return -errno;
                }

                (*cur_pollfdn) -= 1;
            }

            if (pollfds[iter + 1].fd != 1) // close if it is not stdout
            {
                int err = close(pollfds[iter + 1].fd);
                if (err != 0)
                {
                    ERR(fprintf(stderr, "close() failed: %s \n", strerror(errno)));
                    return -errno;
                }

                (*cur_pollfdn) -= 1;
            }

            DBG(fprintf(stderr, "closed %d and %d, pollfd_active == %d \n", 
                                pollfds[iter].fd, pollfds[iter + 1].fd, *cur_pollfdn));

            pollfds[iter].    fd = 0;
            pollfds[iter + 1].fd = 0;

            pollfds[iter].    events = 0;
            pollfds[iter + 1].events = 0;
        }
        else // read nonnegative number of bytes 
        {
            buffers[buffer_ind].cnt = read_ct;
            pollfds[iter].events = 0; 

            pollfds[iter + 1].events = POLLOUT;
            
            DBG(fprintf(stderr, "pollfds[%d].events now is %d "
                            "and pollfds[%d].events now is %d \n", iter,     pollfds[iter].events, 
                                                                    iter + 1, pollfds[iter + 1].events));
        }
    }
    else // buffer is not free 
    {
        DBG(fprintf(stderr, "buffer is not empty \n"));
        pollfds[iter].events = 0; // do not poll events cause buffer is not empty
    }

    pollfds[iter].revents = 0;

    return 0;
}
//---------------------------------------------------------

static void setup_start_pollfds(struct Child_pipe* child_pipes, unsigned prog_times, struct pollfd* pollfds)
{
    assert(child_pipes);
    assert(pollfds);

    unsigned cur_pollfd = 0;

    INIT_POLLFD(pollfds, cur_pollfd, 0, POLLIN, 0);
    cur_pollfd++;

    for (unsigned iter = 0; iter < prog_times; iter++)
    {
        INIT_POLLFD(pollfds, cur_pollfd, child_pipes[iter].fdin, 0, 0);
        cur_pollfd++;

        INIT_POLLFD(pollfds, cur_pollfd, child_pipes[iter].fdout, 0, 0);
        cur_pollfd++;   
    }

    INIT_POLLFD(pollfds, cur_pollfd, 1, 0, 0);

    for (unsigned iter = 0; iter < prog_times * 2 + 2; iter++)
    {
        DBG(fprintf(stderr, "pollfds[%d]: fd == %d; events == %d; revents  == %d; \n", iter, 
                                                                               pollfds[iter].fd, 
                                                                               pollfds[iter].events, 
                                                                               pollfds[iter].revents));
    }

    return;
}

//---------------------------------------------------------

static int execute_check(struct Child_pipe* child_pipes, unsigned prog_times)
{
    assert(child_pipes);

    struct pollfd* pollfds = (struct pollfd*) calloc(prog_times * 2 + 2, sizeof(struct pollfd));
    //                                               ^^^^^^^^^^^^^^^^^^
    //                                               2 (write-end and read-end) for each of pipes 
    //                                               and 2 for stdin and stdout of parent process

    assert(pollfds);

    setup_start_pollfds(child_pipes, prog_times, pollfds);

    struct Buffer* buffers = alloc_buffers(prog_times);
    assert(buffers);

    int check_ok = perform_io(child_pipes, prog_times, pollfds, buffers);

    if (check_ok < 0)
    {
        fprintf(stderr, "Error occured duting check. \n");
        return -1;
    }
    else if (check_ok == 1)
    {
        fprintf(stderr, "Check passed successfully. \n");
    }
    else if (check_ok == 0)
    {
        fprintf(stderr, "Check failed successfully XD. \n");
    }

    free_buffers(buffers, prog_times);
    free(pollfds);

    return 0;
}

//---------------------------------------------------------


