#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUF_SIZE 128
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

    current_time(time_str, sizeof(time_str));
    snprintf(buffer, BUF_SIZE, "Writer time: %s, PID: %d", time_str, getpid());

    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (write(fd, buffer, strlen(buffer)) == -1)
    {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);

    unlink(FIFO_PATH);
    return 0;
}
