
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define PROGRAM_NAME "mycat"

int main(int argc, char** argv)
{
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

    /* Getting the file name if provided */

    static char const *infile;
    infile = "-";
    if (optind < argc)
        infile = argv[optind];
    
    /* Reading the file */
    FILE *fp = fopen(infile, "r");
    char line;

    if (fp == NULL)
    {
        printf("Unable to open file or filed doesn's exist.");
        exit(EXIT_FAILURE);
    }

    char* endline = (show_ends?"    \n":"\n");
    int linenum = 0;
    char dest[2], line_start[20];
    char* buffer = malloc(sizeof(char)*20);
    char* buffer_original = buffer;
    bool is_eol = false;
    bool next_numbered = false;

    while((line = fgetc(fp)) != EOF)
    {
        dest[0] = line;
        dest[1] = '\0';
        char* temp = buffer;

        if(line == '\n') is_eol = true;
    
        if(number)
        {
            if(linenum == 0)
            {
                buffer = stpcpy(buffer, "0 ");
                linenum++;
            }
            if(next_numbered)
            {
                sprintf(line_start, "%6d  ", linenum);
                buffer = stpcpy(buffer, line_start);
                linenum++;
                next_numbered = false;
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
        printf("%s", temp);
        is_eol = false;
        buffer = buffer_original;
    }
    free(buffer);
    fclose(fp);
    return 0;
}