#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#define PROGRAM_NAME "myls"

typedef struct
{
    mode_t mode;
    nlink_t links;
    char* owner;
    char* group;
    off_t size;
    time_t mod_time;
    char* name;
    char linkname[512];
    blkcnt_t blocks;

} content;

content process_piece(struct dirent *entry, char* path)
{
    content piece;
    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
    struct stat st;
        if (lstat(fullpath, &st) == -1) {
            fprintf(stderr, "%s: Error reading stats of a file", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }
    piece.name = entry->d_name;
    piece.size = st.st_size;
    piece.mode = st.st_mode;
    piece.blocks = st.st_blocks;
    piece.owner = getpwuid(st.st_uid)->pw_name;
    piece.group = getgrgid(st.st_gid)->gr_name;
    piece.links = st.st_nlink;

    if(S_ISLNK(st.st_mode))
    {
        ssize_t n = readlink(fullpath, piece.linkname, sizeof(piece.linkname) - 1);
        if (n == -1) {
            perror("readlink");
        } else {
            piece.linkname[n] = '\0';
        }
    }
    piece.mod_time = st.st_mtime;
    return piece;
};

#include <sys/stat.h>

void get_mode_string(mode_t mode, char out[11])
{
    if      (S_ISREG(mode))  out[0] = '-';
    else if (S_ISDIR(mode))  out[0] = 'd';
    else if (S_ISLNK(mode))  out[0] = 'l';
    else if (S_ISCHR(mode))  out[0] = 'c';
    else if (S_ISBLK(mode))  out[0] = 'b';
    else if (S_ISFIFO(mode)) out[0] = 'p';
    else if (S_ISSOCK(mode)) out[0] = 's';
    else                     out[0] = '?';

    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';

    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';

    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';

    out[10] = '\0';
}


void print_long(content* content_list, int len)
{
    for(int i = 0; i < len; i ++)
    {  
        content piece = content_list[i];
        char* color = "";
        char link_string[2048];
        char mode_string[11];
        struct tm *tm_info = localtime(&piece.mod_time);
        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm_info);
        get_mode_string(piece.mode, mode_string);
        if(piece.mode & (S_IXUSR | S_IXGRP | S_IXOTH)) color = "\e[1;32m";
        if(S_ISDIR(piece.mode)) color = "\e[1;34m";
        if(S_ISLNK(piece.mode))
        {
            color = "\e[1;36m";
            snprintf(link_string, sizeof(link_string), " -> %s", piece.linkname);
            printf("%s %1ld %s %s %5ld %s %s%s\033[0m%s \n", mode_string, piece.links , piece.owner, piece.group, piece.size, timebuf, color, piece.name, link_string);
        }
        else {
            printf("%s %1ld %s %s %5ld %s %s%s\033[0m \n", mode_string, piece.links, piece.owner, piece.group, piece.size, timebuf, color, piece.name);
        }
    }
}

int compare_by_name(const void *a, const void *b) {
    const content *fa = a;
    const content *fb = b;
    return strcmp(fa->name, fb->name);
}


int main(int argc, char** argv)
{
    /* cl arguments*/
    bool long_listing = false;
    bool list_all = false;
    
    /* getopting the arguments*/
    int c;
    while ((c = getopt(argc, argv, "la")) != -1)
        {
        switch (c)
            {
            case 'l':
                long_listing = true;
                break;

            case 'a':
                list_all = true;
                break;

            default:
                fprintf(stderr, "%s: Unknown option: -%c\n", PROGRAM_NAME, optopt);
                exit(EXIT_FAILURE);
            }
        }
    
    DIR *directory = opendir((optind < argc ? argv[argc-1]:"."));

    if (directory == NULL) {
        fprintf(stderr, "%s: cannot access '%s': No such file or directory\n", PROGRAM_NAME, argv[argc-1]);
        return 0;
    }

    struct dirent *entry;
    errno = 0;

    content *content_list = NULL;
    size_t cap = 0;
    size_t len = 0;
    blkcnt_t total_blocks = 0;

    while ((entry = readdir(directory)) != NULL) {
        if (len + 1 >= cap)
        {
            size_t new_cap = cap ? cap * 2 : 1;
            content *new_content_list = realloc(content_list, new_cap*sizeof(content));
            if (new_content_list == NULL)
            {
                fprintf(stderr, "%s: out of memory\n", PROGRAM_NAME);
                free(content_list);
                closedir(directory);
                exit(EXIT_FAILURE);
            }
            content_list = new_content_list;
            cap = new_cap;
        }
        if (!list_all && entry->d_name[0] == '.') {
            continue;
        }
        content_list[len] = process_piece(entry, (optind < argc ? argv[argc-1]:"."));
        total_blocks += content_list[len].blocks;
        len++;
    }
    /* print all */

    qsort(content_list, len, sizeof(content), compare_by_name);


    if(long_listing)
    {
        printf("total: %ld\n", total_blocks/2);
        print_long(content_list, len);
    }
    else
    {
        for(size_t i = 0; i < len; i++)
        {
            char* color = "";
            content piece = content_list[i];
            if(piece.mode & (S_IXUSR | S_IXGRP | S_IXOTH)) color = "\e[1;32m";
            if(S_ISDIR(piece.mode)) color = "\e[1;34m";
            if(S_ISLNK(piece.mode)) color = "\e[1;36m";
            printf("%s%s\033[0m  ", color, content_list[i].name);
        }
        printf("\n");
    }
    

    if (errno != 0) {
        fprintf(stderr, "%s: Error reading contents of the directory", PROGRAM_NAME);
    }

    closedir(directory);
    return 0;
}
