
#include <assert.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>

//---------------------------------------------------------

#include "../../include/my_id/my_id.h"

//=========================================================

static int show_self_id(void);

static int show_other_id(const char* uname);

static int show_uid(uid_t uid);

static int show_gid(gid_t gid);

//=========================================================

static int show_other_id(const char* uname)
{
    assert(uname);

    struct passwd* passwd_entry = getpwnam(uname);
    if (!passwd_entry)
    {
        fprintf(stderr, "getpwnam() syscall failed: %s: %s\n", uname, strerror(errno));
        return errno;
    }

    printf("uid=%d(%s) gid=", passwd_entry->pw_uid, uname);

    int show_gid_err = show_gid(passwd_entry->pw_gid);
    if (show_gid_err)
        return show_gid_err;

    printf(" groups=");

    int ngroups = 0;
    getgrouplist(uname, passwd_entry->pw_gid, NULL, &ngroups);

    if (ngroups == 1)
    {
        int err = show_gid(passwd_entry->pw_gid);
        if (err)
            return err;
    }
    else
    {
        gid_t* groups = (gid_t*) calloc(ngroups, sizeof(gid_t));
        if (!groups)
        {
            fprintf(stderr, "calloc() failed: %s\n", strerror(errno));
            return errno;
        }

        int getgrouplist_ret = getgrouplist(uname, passwd_entry->pw_gid, groups, &ngroups);
        if (getgrouplist_ret == -1)
        {
            fprintf(stderr, "getgrouplist() syscall failed: %s\n", strerror(errno));
            return errno;
        }

        for (unsigned iter = 0; iter < ngroups; iter++)
        {
            gid_t cur_gid = groups[iter];

            int err = show_gid(cur_gid);
            if (err)
                return err;

            if (iter != ngroups - 1)
                putchar(',');
        }

        free(groups);
    }

    putchar('\n');

    return 0;
}

//---------------------------------------------------------

static int show_uid(uid_t uid)
{
    struct passwd* passwd_entry = getpwuid(uid);
    if (!passwd_entry)
    {
        fprintf(stderr, "getpwuid() syscall failed: uid=%d: %s\n", uid, strerror(errno));
        return errno;
    }

    printf("%d(%s)", (int)uid, passwd_entry->pw_name);

    return 0;
}

//---------------------------------------------------------

static int show_gid(gid_t gid)
{
    struct group* group_entry = getgrgid(gid);
    if (!group_entry)
    {
        fprintf(stderr, "getgrgid() syscall failed: gid=%d: %s\n", gid, strerror(errno));
        return errno;
    }

    printf("%d(%s)", (int)gid, group_entry->gr_name);

    return 0;
}

//---------------------------------------------------------

static int show_self_id(void)
{
    uid_t uid = getuid();

    printf("uid=");

    int show_uid_err = show_uid(uid);
    if (show_uid_err)
        return show_uid_err;

    gid_t gid = getgid();

    printf(" gid=");

    int show_gid_err = show_gid(gid);
    if (show_gid_err)
        return show_gid_err;

    int groups_num = getgroups(0, NULL);
    if (groups_num == -1)
    {
        fprintf(stderr, "getgroups() syscall failed: %s\n", strerror(errno));
        return errno;
    }

    printf(" groups=");

    if (groups_num == 1)
    {
        int err = show_gid(gid);
        if (err)
            return err;
    }
    else
    {
        gid_t* list = (gid_t*) calloc(groups_num, sizeof(gid_t));
        if (!list)
        {
            fprintf(stderr, "calloc() failed: %s\n", strerror(errno));
            return errno;
        }

        int sup_grps = getgroups(groups_num, list);
        if (sup_grps == -1 || sup_grps == 0)
        {
            fprintf(stderr, "getgroups() syscall failed: %s\n", strerror(errno));
            return errno;
        }

        for (unsigned iter = 0; iter < groups_num; iter++)
        {
            gid_t cur_gid = list[iter];

            int err = show_gid(cur_gid);
            if (err)
                return err;

            if (iter != groups_num - 1)
                putchar(',');
        }

        free(list);
    }
    
    putchar('\n');

    return 0;
}

//---------------------------------------------------------

int my_id(const int argc, const char** argv)
{
    assert(argv);

    if (argc == 1)
    {
        return show_self_id();
    }
    else
    {
        for (unsigned cur_arg = 1; cur_arg < argc; cur_arg++)
        {
            int err = show_other_id(argv[cur_arg]);
            if (err)
                return err;
        }
    }

    return 0;
}

//=========================================================
