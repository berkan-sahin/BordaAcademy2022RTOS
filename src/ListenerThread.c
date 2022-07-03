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

    while (1) {
        int input;
        char msg[2]; /* One char for the character, one char for \0 */

        pthread_mutex_lock(args->stdin_mutex);
        input = getc(stdin);
        pthread_mutex_unlock(args->stdin_mutex);

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

    return NULL;
}
