#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#define PROGRAM_NAME "Fork demo"

pid_t child_pid = -1;

void exit_message()
{
    switch(child_pid)
    {
    case -1:
        perror("fork");
    case 0:
        printf("Child exiting\n");
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
    child_pid = fork();
    if(child_pid)
    {
        while ((wpid = wait(&status)) > 0)
        {
            printf("Child PID-%d's exit value: %d\n", wpid, status);
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