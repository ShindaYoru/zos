#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <regex.h>

#define PROGRAM_NAME "mygrep"

int main(int argc, char** argv)
{
    int exit_status = 1;
    bool matched = false;
    bool had_error = false;

    /* Command line arguments */
    if (argc < 2)
    {
        fprintf(stderr, "%s: missing pattern\n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }


    /* Checking regex */
    const char *pattern = argv[1];
    regex_t regex;
    int regcomp_status = regcomp(&regex, pattern, 0);
    if (regcomp_status != 0)
    {
        char errbuf[256];
        regerror(regcomp_status, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "%s: %s\n", PROGRAM_NAME, errbuf);
        exit(EXIT_FAILURE);
    }

    int first_file = 2;
    int file_count = argc - first_file;
    bool multiple_files = file_count > 1;

    /* When no filename is provided stdin will be read */
    if (file_count == 0)
    {
        static char *stdin_argv[] = { "-", NULL };
        argv = stdin_argv;
        argc = 1;
        first_file = 0;
    }

    /* Main loop */
    for (int argi = first_file; argi < argc; argi++)
    {
        const char *infile = argv[argi];
        const char *display_name = (strcmp(infile, "-") == 0) ? "(standard input)" : infile;
        FILE *fp = (strcmp(infile, "-") ? fopen(infile, "r") : stdin);

        if (fp == NULL)
        {
            fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, display_name, strerror(errno));
            had_error = true;
            exit_status = 2;
            continue;
        }

        char *line = NULL;
        size_t cap = 0;
        size_t len = 0;
        int chr;

        /* Reading each character in the file and checking for EOF */
        while ((chr = fgetc(fp)) != EOF)
        {
            if (len + 1 >= cap)
            {
                size_t new_cap = cap ? cap * 2 : 128;
                char *new_line = realloc(line, new_cap);
                if (new_line == NULL)
                {
                    fprintf(stderr, "%s: out of memory\n", PROGRAM_NAME);
                    free(line);
                    if (fp != stdin) fclose(fp);
                    regfree(&regex);
                    exit(EXIT_FAILURE);
                }
                line = new_line;
                cap = new_cap;
            }

            line[len++] = chr;

            if (chr == '\n')
            {
                line[len] = '\0';

                if (regexec(&regex, line, 0, NULL, 0) == 0)
                {
                    if (multiple_files)
                        printf("%s:", display_name);

                    fputs(line, stdout);
                    matched = true;
                }

                len = 0;
            }
        }

        /* Handle last line without \n */
        if (len > 0)
        {
            line[len] = '\0';

            if (regexec(&regex, line, 0, NULL, 0) == 0)
            {
                if (multiple_files)
                    printf("%s:", display_name);

                fputs(line, stdout);
                matched = true;
            }
        }

        free(line);
        if (fp != stdin) fclose(fp);
    }

    regfree(&regex);

    if (had_error)
        exit_status = 2;
    else if (matched)
        exit_status = 0;
    else
        exit_status = 1;

    return exit_status;
}