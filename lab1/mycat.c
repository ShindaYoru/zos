
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define PROGRAM_NAME "mycat"

int main(int argc, char** argv)
{
    int exit_status = EXIT_SUCCESS;
    
    /* Command line arguments */
    bool number = false;
    bool number_nonblank = false;
    bool show_ends = false;

    /* Parsing command line arguments */

    int c;
    while ((c = getopt(argc, argv, "bnE")) != -1)
        {
        switch (c)
            {
            case 'b':
                number = true;
                number_nonblank = true;
                break;

            case 'n':
                number = true;
                break;

            case 'E':
                show_ends = true;
                break;

            default:
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                exit(EXIT_FAILURE);
            }
        }

    /* When no filename is provided stdin (-) will be read */
    static char const *infile;
    infile = "-";
    int linenum = 1;
    bool is_eol = false;
    bool next_numbered = true;
    if (optind >= argc) {
        static char *stdin_argv[] = { "-", NULL };
        optind = 0;
        argc = 1;
        argv = stdin_argv;
    }
    /* Main loop */
    while (optind < argc) {
        infile = argv[optind];
    
        /* Reading the file */
        FILE *fp = (strcmp(infile, "-") ? fopen(infile, "r") : stdin);
        int chr;

        if (fp == NULL)
        {
            fprintf(stderr, "%s: %s\n", infile, strerror(errno));
            exit_status = EXIT_FAILURE;
            optind++;
            continue;
        }

        char dest[2], line_start[20];
        char* buffer = malloc(sizeof(char)*25);
        char* buffer_original = buffer;

        /* Reading each character in the file and checking for EOF */
        while((chr = fgetc(fp)) != EOF)
        {
            dest[0] = chr;
            dest[1] = '\0';

            if(chr == '\n') is_eol = true;
        
            if(number)
            {
                if(next_numbered)
                {
                    if(!(number_nonblank && is_eol)) {
                        sprintf(line_start, "%6d\t", linenum);
                        buffer = stpcpy(buffer, line_start);
                        linenum++;
                        next_numbered = false;
                    }
                }

                if(is_eol) next_numbered = true;
            }

            if(show_ends)
            {
                if(is_eol)
                {
                    buffer = stpcpy(buffer, "$");
                }
            }
            buffer = stpcpy(buffer, dest);
            printf("%s", buffer_original);
            is_eol = false;
            buffer = buffer_original;
        }
        free(buffer);
        if (fp != stdin) fclose(fp);
        optind++;
    }
    return exit_status;
}