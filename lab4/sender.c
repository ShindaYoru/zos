#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PROGRAM_NAME "prak7_sender"
#define SHM_FILE     "/tmp/prak7_shm_key"
#define PID_FILE     "/tmp/prak7_sender.pid"
#define PROJ_ID      'P'
#define MSG_SIZE     128

typedef struct
{
    volatile unsigned seq;
    char message[MSG_SIZE];
} shared_data;

int main(int argc, char** argv)
{
    int exit_status = EXIT_SUCCESS;

    /* Command line arguments */
    (void)argc;
    (void)argv;

    /* Checking that sender is not already running */
    FILE *pid_fp = fopen(PID_FILE, "r");
    if (pid_fp != NULL)
        {
        int other_pid = 0;
        int n = fscanf(pid_fp, "%d", &other_pid);
        fclose(pid_fp);
        if (n == 1 && other_pid > 0 && kill((pid_t)other_pid, 0) == 0)
            {
            fprintf(stderr, "%s: sender is already running (pid %d)\n",
                    PROGRAM_NAME, other_pid);
            exit(EXIT_FAILURE);
            }
        }

    /* Saving our pid to the file */
    pid_fp = fopen(PID_FILE, "w");
    if (pid_fp == NULL)
        {
        fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, PID_FILE, strerror(errno));
        exit(EXIT_FAILURE);
        }
    fprintf(pid_fp, "%d\n", (int)getpid());
    fclose(pid_fp);

    /* Preparing the marker file for ftok() */
    int fd = open(SHM_FILE, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
        {
        fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, SHM_FILE, strerror(errno));
        exit(EXIT_FAILURE);
        }
    close(fd);

    /* Creating the shared memory segment */
    key_t key = ftok(SHM_FILE, PROJ_ID);
    if (key < 0)
        {
        fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, SHM_FILE, strerror(errno));
        exit(EXIT_FAILURE);
        }

    int shm_id = shmget(key, sizeof(shared_data), IPC_CREAT | 0666);
    if (shm_id < 0)
        {
        fprintf(stderr, "%s: shmget: %s\n", PROGRAM_NAME, strerror(errno));
        exit(EXIT_FAILURE);
        }

    shared_data *shm_ptr = (shared_data *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1)
        {
        fprintf(stderr, "%s: shmat: %s\n", PROGRAM_NAME, strerror(errno));
        exit(EXIT_FAILURE);
        }

    shm_ptr->seq = 0;
    shm_ptr->message[0] = '\0';

    /* Main loop */
    while (true)
        {
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_r(&now, &tm_now);

        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm_now);

        unsigned seq = shm_ptr->seq;
        shm_ptr->seq = seq + 1;

        snprintf(shm_ptr->message, MSG_SIZE, "time=%s pid=%d",
                 timebuf, (int)getpid());

        shm_ptr->seq = seq + 2;

        sleep(1);
        }

    return exit_status;
}