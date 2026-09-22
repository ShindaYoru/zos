#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define PROGRAM_NAME "mychmod"
#define MAX_CLAUSES 32

typedef struct
{
    bool who[3];    /* [0]=u, [1]=g, [2]=o */
    char op;        /* '+', '-', '=' */
    bool perm[3];   /* [0]=r, [1]=w, [2]=x */
    bool setid[3];  /* [0]=setuid (u+s), [1]=setgid (g+s), [2]=sticky (o+t) */
    bool who_given; 
    bool capital_x; 
} symbolic_op;


static bool parse_octal(const char *s, mode_t *out)
{
    if (s == NULL || *s == '\0')
        return false;

    char *end;
    errno = 0;
    unsigned long value = strtoul(s, &end, 8);

    if (*end != '\0' || errno != 0 || value > 07777)
        return false;

    *out = (mode_t)value;
    return true;
}

static bool parse_symbolic(const char *s, symbolic_op *out)
{
    const char *p = s;

    memset(out, 0, sizeof(*out));
    out->op = '\0';

    while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a')
    {
        if (*p == 'u') out->who[0] = true;
        if (*p == 'g') out->who[1] = true;
        if (*p == 'o') out->who[2] = true;
        if (*p == 'a')
        { 
            out->who[0] = true;
            out->who[1] = true;
            out->who[2] = true;
        }
        p++;
    }
    out->who_given = (p != s);

    if (!out->who_given){
        out->who[0] = true;
        out->who[1] = true;
        out->who[2] = true;
    }

    if (*p != '+' && *p != '-' && *p != '=')
        return false;
    out->op = *p;
    p++;

    while (*p == 'r' || *p == 'w' || *p == 'x' || *p == 'X' || *p == 's' || *p == 't')
    {
        if (*p == 'r') out->perm[0] = true;
        if (*p == 'w') out->perm[1] = true;
        if (*p == 'x') out->perm[2] = true;
        if (*p == 'X') out->capital_x = true;

        if (*p == 's')
        {
            if (out->who_given)
            {
                if (out->who[0]) out->setid[0] = true;
                if (out->who[1]) out->setid[1] = true;
            }
            else
            {
                out->setid[0] = true;
                out->setid[1] = true;
            }
        }
        if (*p == 't')
        {
            if (!out->who_given || out->who[2])
                out->setid[2] = true;
        }
        p++;
    }

    if (*p != '\0')
        return false;

    if (!out->perm[0] && !out->perm[1] && !out->perm[2] && !out->capital_x &&
        !out->setid[0] && !out->setid[1] && !out->setid[2] && out->op != '=')
        return false;

    return true;
}

static int parse_symbolic_clauses(const char *mode_arg, symbolic_op *out, int max_clauses)
{
    char *copy = strdup(mode_arg);
    if (copy == NULL)
    {
        fprintf(stderr, "%s: out of memory\n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char *saveptr;
    char *token = strtok_r(copy, ",", &saveptr);

    while (token != NULL)
    {
        if (count >= max_clauses)
        {
            fprintf(stderr, "%s: too many clauses in mode: %s\n", PROGRAM_NAME, mode_arg);
            free(copy);
            return -1;
        }
        if (!parse_symbolic(token, &out[count]))
        {
            free(copy);
            return -1;
        }
        count++;
        token = strtok_r(NULL, ",", &saveptr);
    }

    free(copy);

    if (count == 0)
        return -1;

    return count;
}


static mode_t apply_symbolic(mode_t mode, const symbolic_op *op, mode_t umask_val)
{
    mode_t bits[3][3] = {
        { S_IRUSR, S_IWUSR, S_IXUSR },
        { S_IRGRP, S_IWGRP, S_IXGRP },
        { S_IROTH, S_IWOTH, S_IXOTH }
    };
    mode_t idbits[3] = { S_ISUID, S_ISGID, S_ISVTX };

    bool x_allowed = S_ISDIR(mode) ||
                     (mode & (S_IXUSR | S_IXGRP | S_IXOTH));

    for (int c = 0; c < 3; c++)
    {
        if (!op->who[c])
            continue;

        mode_t mask = 0;
        if (op->perm[0]) mask |= bits[c][0];
        if (op->perm[1]) mask |= bits[c][1];
        if (op->perm[2] || (op->capital_x && x_allowed))
            mask |= bits[c][2];

        if (!op->who_given && op->op != '=')
            mask &= ~umask_val;

        switch (op->op)
        {
        case '+':
            mode |= mask;
            break;
        case '-':
            mode &= ~mask;
            break;
        case '=':
            mode &= ~(bits[c][0] | bits[c][1] | bits[c][2]);
            mode |= mask;
            break;
        }
    }

    for (int c = 0; c < 3; c++)
    {
        if (!op->who[c])
            continue;

        if (op->op == '=')
        {
            mode &= ~idbits[c];
            if (op->setid[c])
                mode |= idbits[c];
        }
        else if (op->setid[c])
        {
            if (op->op == '+')
                mode |= idbits[c];
            else 
                mode &= ~idbits[c];
        }
    }

    return mode;
}

int main(int argc, char** argv)
{
    int exit_status = EXIT_SUCCESS;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s MODE file...\n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    const char *mode_arg = argv[1];
    bool symbolic = false;

    symbolic_op sops[MAX_CLAUSES];
    int sop_count = 0;
    mode_t octal_mode = 0;

    if (parse_octal(mode_arg, &octal_mode))
    {
        symbolic = false;
    }
    else
    {
        sop_count = parse_symbolic_clauses(mode_arg, sops, MAX_CLAUSES);
        if (sop_count < 0)
        {
            fprintf(stderr, "%s: invalid mode: %s\n", PROGRAM_NAME, mode_arg);
            exit(EXIT_FAILURE);
        }
        symbolic = true;
    }

    mode_t old_umask = umask(0);
    umask(old_umask);

    /* Main loop */
    for (int argi = 2; argi < argc; argi++)
    {
        const char *infile = argv[argi];

        struct stat st;
        if (stat(infile, &st) != 0)
        {
            fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, infile, strerror(errno));
            exit_status = EXIT_FAILURE;
            continue;
        }

        mode_t new_mode;
        if (symbolic)
        {
            new_mode = st.st_mode;
            for (int i = 0; i < sop_count; i++)
                new_mode = apply_symbolic(new_mode, &sops[i], old_umask);
        }
        else
        {
            new_mode = octal_mode;
        }

        if (chmod(infile, new_mode) != 0)
        {
            fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, infile, strerror(errno));
            exit_status = EXIT_FAILURE;
            continue;
        }
    }

    return exit_status;
}
