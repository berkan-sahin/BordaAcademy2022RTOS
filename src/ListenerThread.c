//
// Created by ayupa on 1/07/2022.
//

#include "ListenerThread.h"
#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

typedef struct {
    mqd_t block_queue;
    mqd_t stream_queue;
    sem_t *block_sem;
    sem_t *stream_sem;
    const char* block_mq_name;
    const char* stream_mq_name;
    const char* block_sem_name;
    const char* stream_sem_name;
} ListenerCleanup_arg_t;

void ListenerThreadCleanup(void *arg) {
    ListenerCleanup_arg_t *args = (ListenerCleanup_arg_t*) arg;
    mq_close(args->block_queue);
    mq_close(args->stream_queue);
    mq_unlink(args->block_mq_name);
    mq_unlink(args->stream_mq_name);

    sem_close(args->block_sem);
    sem_close(args->stream_sem);
    sem_unlink(args->block_sem_name);
    sem_unlink(args->stream_sem_name);


}

void *ListenerThread(void *arg) {
    ListenerThread_arg_t *args = (ListenerThread_arg_t *) arg;
    mqd_t block_queue, stream_queue;
    sem_t *block_sem, *stream_sem;

    block_queue = mq_open(args->block_mq_name, O_CREAT | O_WRONLY,
                          S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                          args->queue_attrs);
    stream_queue = mq_open(args->stream_mq_name, O_CREAT | O_WRONLY,
                           S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                           args->queue_attrs);

    if (block_queue == -1 || stream_queue == -1) {
        pthread_mutex_lock(args->stderr_mutex);
        perror("ListenerThread 1");
        pthread_mutex_unlock(args->stderr_mutex);
        pthread_exit(NULL);
    }

    block_sem = sem_open(args->block_sem_name, O_CREAT | O_WRONLY,
                         S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                         0);
    stream_sem = sem_open(args->stream_sem_name, O_CREAT | O_WRONLY,
                          S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                          0);

    if (block_sem == SEM_FAILED || stream_sem == SEM_FAILED) {
        pthread_mutex_lock(args->stderr_mutex);
        perror("ListenerThread 2");
        pthread_mutex_unlock(args->stderr_mutex);

        mq_close(block_queue);
        mq_close(stream_queue);
        pthread_exit(NULL);
    }

    /* Initialize cleanup handler */
    ListenerCleanup_arg_t cleanupArg = {
            .block_sem_name = args->block_sem_name,
            .block_sem = block_sem,
            .stream_sem_name = args->stream_sem_name,
            .stream_sem = stream_sem,
            .block_queue = block_queue,
            .block_mq_name = args->block_mq_name,
            .stream_mq_name = args->stream_mq_name,
            .stream_queue = stream_queue
    };

    pthread_cleanup_push(ListenerThreadCleanup, &cleanupArg);

    while (1) {
        int input;
        char msg[2]; /* One char for the character, one char for \0 */

        pthread_mutex_lock(args->stdin_mutex);
        input = getc(stdin);
        pthread_mutex_unlock(args->stdin_mutex);

        /* Check if the input char is valid.
         * Valid chars are 0-9, A-Z, _ and space.
         */
        if (isalnum(input) || input == '_' || input == ' ') {

            snprintf(msg, 2, "%c", input);

            if (mq_send(block_queue, msg, strlen(msg), 0) != 0) {
                if (errno != EAGAIN) { /* Don't terminate the thread if the queue is full */

                    pthread_mutex_lock(args->stderr_mutex);
                    perror("ListenerThread 3");
                    pthread_mutex_unlock(args->stderr_mutex);

                    mq_close(block_queue);
                    mq_close(stream_queue);
                    pthread_exit(NULL);
                }
            }

            sem_post(block_sem);

            if (mq_send(stream_queue, msg, strlen(msg), 0) != 0) {
                if (errno != EAGAIN) { /* Don't terminate the thread if the queue is full */

                    pthread_mutex_lock(args->stderr_mutex);
                    perror("ListenerThread 4");
                    pthread_mutex_unlock(args->stderr_mutex);

                    mq_close(block_queue);
                    mq_close(stream_queue);
                    pthread_exit(NULL);
                }
            }

            sem_post(stream_sem);
        }
    }

    pthread_cleanup_pop(1);
    return NULL;
}
