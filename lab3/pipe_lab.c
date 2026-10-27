#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

int main() 
{

    /* Make child and wait */
    int status = 0;
    int fd1[2];
    pid_t wpid;
    
    fflush(NULL);
    pid_t child_pid = fork();
    if(child_pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pipe(fd1) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    if(child_pid > 0)
    {
        while ((wpid = wait(&status)) > 0)
        {
            if (WIFEXITED(status))
                printf("Child exited with code %d\n", WEXITSTATUS(status));
            else if (WIFSIGNALED(status))
                printf("Child killed by signal %d\n", WTERMSIG(status));
        }
    }
    /* Main body*/
    pid_t my_pid = getpid();
    pid_t parent_pid = getppid();
    printf("PID: %d\tPPID: %d\n", my_pid, parent_pid);

    return 0;
}