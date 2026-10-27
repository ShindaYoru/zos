#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUF_SIZE 128
#define DELAY_SEC 5
#define FIFO_PATH "lab6_fifo"

/* Write current time as HH:MM:SS into buf */
void current_time(char *buf, size_t size)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (tm_info == NULL || strftime(buf, size, "%H:%M:%S", tm_info) == 0)
    {
        snprintf(buf, size, "time:error");
    }
}

int main()
{
    char time_str[32];
    char buffer[BUF_SIZE];

    if (mkfifo(FIFO_PATH, 0600) == -1 && errno != EEXIST)
    {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    sleep(DELAY_SEC);
    
    ssize_t n = read(fd, buffer, BUF_SIZE - 1);
    if (n == -1)
    {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    close(fd);

    current_time(time_str, sizeof(time_str));
    printf("Reader time: %s\n", time_str);
    printf("Reader received: %s\n", buffer);
    return 0;
}
