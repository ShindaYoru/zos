#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PROGRAM_NAME "prak7_receiver"
#define SHM_FILE     "/tmp/prak7_shm_key"
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

    /* Preparing the marker file for ftok() */
    int fd = open(SHM_FILE, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
        {
        fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, SHM_FILE, strerror(errno));
        exit(EXIT_FAILURE);
        }
    close(fd);

    key_t key = ftok(SHM_FILE, PROJ_ID);
    if (key < 0)
        {
        fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, SHM_FILE, strerror(errno));
        exit(EXIT_FAILURE);
        }

    /* Waiting for the sender to create the shared segment */
    int shm_id;
    while (true)
        {
        shm_id = shmget(key, sizeof(shared_data), 0666);
        if (shm_id >= 0)
            break;

        if (errno != ENOENT)
            {
            fprintf(stderr, "%s: shmget: %s\n", PROGRAM_NAME, strerror(errno));
            exit(EXIT_FAILURE);
            }

        struct timespec req = {0, 200000000};
        nanosleep(&req, NULL);
        }

    shared_data *shm_ptr = (shared_data *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1)
        {
        fprintf(stderr, "%s: shmat: %s\n", PROGRAM_NAME, strerror(errno));
        exit(EXIT_FAILURE);
        }

    /* Main loop */
    while (true)
        {
        char local[MSG_SIZE];
        unsigned s1, s2;

        /* Waiting for a consistent snapshot */
        while (true)
            {
            s1 = shm_ptr->seq;
            if (s1 & 1u)
                continue;

            memcpy(local, shm_ptr->message, MSG_SIZE);

            s2 = shm_ptr->seq;
            if (s1 == s2)
                break;
            }

        local[MSG_SIZE - 1] = '\0';

        time_t now = time(NULL);
        struct tm tm_now;
        localtime_r(&now, &tm_now);

        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm_now);

        printf("my_time=%s my_pid=%d received=\"%s\"\n",
               timebuf, (int)getpid(), local);
        fflush(stdout);

        struct timespec req = {0, 500000000};
        nanosleep(&req, NULL);
        }

    return exit_status;
}