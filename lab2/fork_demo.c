#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

pid_t child_pid = -1;
volatile sig_atomic_t got_sigint = 0;
volatile sig_atomic_t got_sigterm = 0;

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

void Signal_Interrupt(int signo)
{
    got_sigint = signo; 
}

void Signal_Terminate(int signo)
{
    got_sigterm = signo;
}

int main() 
{
    /* Handle atexit*/
    int atexit_status = atexit(exit_message);
    if (atexit_status != 0)
    {
        fprintf(stderr, "Cannot set exit function\n");
        exit(EXIT_FAILURE);
    }

    /* Handle signal*/
    if(signal(SIGINT, Signal_Interrupt) == SIG_ERR) 
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = Signal_Terminate;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

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
    if (got_sigterm)
    {
        printf("Received SIGTERM (signal %d): termination request\n", got_sigterm);
        exit(EXIT_FAILURE);
    }
    if(got_sigint)
    {
        printf("Received SIGINT (signal %d): interrupt request\n", got_sigint);
        exit(EXIT_FAILURE);
    }

    /* Main body*/
    pid_t my_pid = getpid();
    pid_t parent_pid = getppid();
    printf("PID: %d\tPPID: %d\n", my_pid, parent_pid);

    return 0;
}