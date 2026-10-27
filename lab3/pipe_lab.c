#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

#define BUF_SIZE 128
#define DELAY_SEC 5

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
    int fd1[2];
    char time_str[32];
    char buffer[BUF_SIZE];

    if (pipe(fd1) == -1) 
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* Make child and wait */
    fflush(NULL);
    pid_t child_pid = fork();
    if(child_pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(child_pid == 0)
    {
        close(fd1[1]);

        sleep(DELAY_SEC);

        ssize_t n = read(fd1[0], buffer, BUF_SIZE - 1);
        if (n == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';
        close(fd1[0]);

        current_time(time_str, sizeof(time_str));
        printf("Child time: %s\n", time_str);
        printf("Child received: %s\n", buffer);
        exit(EXIT_SUCCESS);
    }

    close(fd1[0]);

    current_time(time_str, sizeof(time_str));
    snprintf(buffer, BUF_SIZE, "Parent time: %s, PID: %d", time_str, getpid());
    if (write(fd1[1], buffer, strlen(buffer)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    close(fd1[1]);
    int status = 0;
    while (wait(&status) > 0)
    {
    }
    return 0;
}
