#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

//---------------------------------------------------------

#include "../../inc/my_shell/my_shell.h"
#include "../../inc/parser/parser.h"
#include "../../inc/utility/utility.h"
#include "../../inc/argvec/argvec.h"

//=========================================================

static const int Cmnds_init_cap      = 8;

static const int Cmnds_resize_step   = 2;

//---------------------------------------------------------

static const int Buffer_init_cap     = 1024;
 
static const int Buffer_cap_step     = 1024;

//=========================================================

static int cmnds_push_back(struct Cmnds* cmnds, const struct Cmnds_entry* cmnds_entry);

static int cmnds_increase(struct Cmnds* cmnds);

static int read_to_buffer(struct Cmnds* cmnds, const int fdin);

//---------------------------------------------------------

static int get_command(struct Cmnds* cmnds, int* cur_pos, int command_ct);

static int get_prog_name(char* buffer, int* cur_pos, struct Cmnds_entry* entry);

static int get_prog_args(char* buffer, int* cur_pos, struct Cmnds_entry* entry);

static int get_pipe_sym (char* buffer, int* cur_pos);

static char* get_word(char* buffer, int* cur_pos);

static char* skip_blank(char* buffer, int* cur_pos);

//=========================================================

int cmnds_ctor(struct Cmnds* cmnds)
{
    assert(cmnds);

    cmnds->size = 0;
    cmnds->cap  = Cmnds_init_cap;

    cmnds->buffer = NULL;

    cmnds->data = (struct Cmnds_entry*) calloc(Cmnds_init_cap, sizeof(Cmnds_entry));
    if (cmnds->data == NULL)
    {
        fprintf(stderr, "calloc() failed to allocate mem \n");
        return -1;
    }

    return 0;
}

//---------------------------------------------------------

int cmnds_dtor(struct Cmnds* cmnds)
{
    assert(cmnds);

    if (cmnds->data)
        free(cmnds->data);

    if (cmnds->buffer)
        free(cmnds->buffer);

    cmnds->size = 0;
    cmnds->cap  = 0;

    return 0;
}

//---------------------------------------------------------

static int cmnds_push_back(struct Cmnds* cmnds, const struct Cmnds_entry* cmnds_entry)
{
    assert(cmnds);

    if (cmnds->size >= cmnds->cap)
    {
        int err = cmnds_increase(cmnds);
        if (err < 0)
        {
            fprintf(stderr, "cmnds_increase() failed \n");
            return err;
        }
    }

    int size = cmnds->size;

    cmnds->data[size].prog_name = cmnds_entry->prog_name;
    
    int err = arg_vec_copy_assignment(&(cmnds->data[size].arg_vec), &cmnds_entry->arg_vec);
    if (err < 0)
    {
        fprintf(stderr, "arg_vec_copy_assignment() failed \n");
        return err;
    }

    cmnds->size += 1;

    return 0;
}

//---------------------------------------------------------

static int cmnds_increase(struct Cmnds* cmnds)
{
    assert(cmnds);

    int old_cap = cmnds->cap;
    int new_cap = old_cap * Cmnds_resize_step;
    
    cmnds->data = (struct Cmnds_entry*) my_recalloc(cmnds->data, new_cap, 
                                                                 old_cap, 
                                                                 sizeof(struct Cmnds_entry));
    if (cmnds->data == NULL)
    {
        fprintf(stderr, "my_recalloc() failed \n");
        return -1;
    }

    cmnds->cap = new_cap;

    return 0;
}

//---------------------------------------------------------

int parse_args(struct Cmnds* cmnds, const int fdin)
{
    assert(cmnds);

    int err = read_to_buffer(cmnds, fdin);
    if (err != 0)
        return err;

    int cur_pos    = 0;
    int command_ct = 0;

    err = get_command(cmnds, &cur_pos, command_ct++);
    if (err != 0)
    {
        fprintf(stderr, "get_command() failed \n");
        return err;
    }

    while (cur_pos < cmnds->buf_size - 1)
    {
        err = get_pipe_sym(cmnds->buffer, &cur_pos);
        if (err != 0)
        {
            fprintf(stderr, "get_prog_name() failed \n");
            return err;
        }

        err = get_command(cmnds, &cur_pos, command_ct++);
        if (err != 0)
        {
            fprintf(stderr, "get_command() failed \n");
            return err;
        }
    }

    return 0;
}

//---------------------------------------------------------

static int get_command(struct Cmnds* cmnds, int* cur_pos, int command_ct)
{
    assert(cmnds);
    assert(cur_pos);

    int err = 0;

    struct Cmnds_entry entry = { 0 };

    err = arg_vec_ctor(&entry.arg_vec);
    if (err < 0)
    {
        fprintf(stderr, "arg_vec_ctor() failed  \n");
        return err;
    }

    char* buffer = cmnds->buffer;

    err = get_prog_name(buffer, cur_pos, &entry);
    if (err != 0)
    {
        if (command_ct == 0)
        {
            err = arg_vec_dtor(&entry.arg_vec);
            if (err < 0)
            {
                fprintf(stderr, "arg_vec_dtor() failed  \n");
                return err;
            }

            return 0;
        }
        else 
        {
            fprintf(stderr, "get_prog_name() failed \n");
            return err;
        }
    }

    err = get_prog_args(buffer, cur_pos, &entry);
    if (err != 0)
    {
        fprintf(stderr, "get_prog_name() failed \n");
        return err;
    }

    err = cmnds_push_back(cmnds, &entry);
    if (err != 0)
    {
        fprintf(stderr, "cmnds_push_back() failed \n");
        return err;
    }

    err = arg_vec_dtor(&entry.arg_vec);
    if (err < 0)
    {
        fprintf(stderr, "arg_vec_dtor() failed  \n");
        return err;
    }

    skip_blank(buffer, cur_pos);

    return 0;
}

//---------------------------------------------------------

static int get_prog_name(char* buffer, int* cur_pos, struct Cmnds_entry* entry)
{
    assert(buffer);
    assert(cur_pos);
    assert(entry);

    char* prog_name = get_word(buffer, cur_pos);
    if (prog_name == NULL)
        return -1;

    entry->prog_name = prog_name;

    int err = arg_vec_push_back(&(entry->arg_vec), prog_name);
    if (err < 0)
    {
        fprintf(stderr, "arg_vec_push_back() failed \n");
        return err;
    }

    return 0;
}

//---------------------------------------------------------

static int get_prog_args(char* buffer, int* cur_pos, struct Cmnds_entry* entry)
{
    assert(buffer);
    assert(cur_pos);
    assert(entry);

    while(1)
    {
        char* prog_arg = get_word(buffer, cur_pos);
        if (prog_arg == NULL)
            break;

        int err = arg_vec_push_back(&(entry->arg_vec), prog_arg);
        if (err < 0)
        {
            fprintf(stderr, "arg_vec_push_back() failed \n");
            return err;
        }
    }

    return 0;
}

//---------------------------------------------------------

static int get_pipe_sym (char* buffer, int* cur_pos)
{
    assert(buffer);
    assert(cur_pos);

    char* not_blank = skip_blank(buffer, cur_pos);
    if (*not_blank == '\0')
        return -1;

    if (*not_blank != '|')
        return -1;
    
    *cur_pos += 1;
    return 0;
}

//---------------------------------------------------------

static char* get_word(char* buffer, int* cur_pos)
{
    assert(buffer);
    assert(cur_pos);
    
    char* word_start = skip_blank(buffer, cur_pos);
    if (*word_start == '\0' || *word_start == '|')
        return NULL;

    char ch = 0;

    while((ch = buffer[*cur_pos]) != '\0')
    {
        if (isspace(ch))
            break;
        else
            *cur_pos += 1;
    }

    buffer[*cur_pos] = '\0';
    *cur_pos += 1;

    return word_start;
}

//---------------------------------------------------------

static char* skip_blank(char* buffer, int* cur_pos)
{
    assert(buffer);
    assert(cur_pos);

    char ch = 0;

    while((ch = buffer[*cur_pos]) != '\0')
    {
        if (isspace(ch))
            *cur_pos += 1;
        else
            break;
    }

    return buffer + (*cur_pos);
}

//---------------------------------------------------------

static int read_to_buffer(struct Cmnds* cmnds, const int fdin)
{
    assert(cmnds);

    cmnds->buffer = (char*) calloc(Buffer_init_cap, sizeof(char));
    if (cmnds->buffer == NULL)
    {
        fprintf(stderr, "calloc() failed \n");
        return -1;
    }

    int buf_size = 0;
    int buf_cap  = Buffer_init_cap;

    int read_bytes = read(fdin, cmnds->buffer, buf_cap);
    if (read_bytes == -1)
    {
        fprintf(stderr, "read() syscall failed: %s \n", strerror(errno));
        return -errno;
    }

    buf_size += read_bytes;

    if (read_bytes == buf_cap)
    {
        while (read_bytes != 0)
        {
            int new_cap = buf_cap + Buffer_cap_step;

            cmnds->buffer = (char*) my_recalloc(cmnds->data, new_cap, buf_cap, sizeof(char));
            if (cmnds->buffer == NULL)
            {
                fprintf(stderr, "my_recalloc() failed \n");
                return -1;
            }

            buf_cap = new_cap;

            read_bytes = read(fdin, cmnds->buffer + buf_size, Buffer_cap_step);
            if (read_bytes == -1)
            {
                fprintf(stderr, "read() syscall failed: %s \n", strerror(errno));
                return -errno;
            }

            buf_size += read_bytes;
        }
    }

    cmnds->buf_size = buf_size;
    cmnds->buffer[buf_size] = '\0';

    return 0;
}