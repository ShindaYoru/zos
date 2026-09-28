#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

pid_t child_pid = -1;

void exit_message()
{
    switch(child_pid)
    {
    case 0:
        printf("Child exiting\n");
        break;
    default:
        printf("Child is PID %d\n", child_pid);
        printf("Parent exiting\n");
    }
}

int main() 
{
    /* Make child and wait */
    int status = 0;
    pid_t wpid;
    fflush(NULL);
    child_pid = fork();
    if(child_pid == -1)
    {
        perror("fork");
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

    /* Handle atexit*/
    int atexit_status = atexit(exit_message);
    if (atexit_status != 0)
    {
        fprintf(stderr, "Cannot set exit function\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}