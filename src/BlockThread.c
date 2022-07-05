//
// Created by bsahin on 5/07/22.
//

#include "BlockThread.h"
#include <semaphore.h>
#include <stdio.h>

typedef struct {
    const char *mq_name;
    const char *queue_sem_name;
    const char *buffer_sem_name;
    mqd_t mq_handle;
    sem_t *queue_sem_handle;
    sem_t *buffer_sem_handle;
} cleanup_arg_t;

void BlockThreadCleanup(void *arg) {
    cleanup_arg_t *args = (cleanup_arg_t*) arg;
    /* TODO */
}

void *BlockThread(void *arg) {
    BlockThread_arg_t *args = (BlockThread_arg_t*) arg;

    mqd_t input_mq;
    sem_t *mq_sem, *buffer_sem;
    int i = 0; /* buffer index */

    input_mq = mq_open(args->mq_name, O_CREAT | O_RDONLY,
                       S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                       args->queue_attrs);

    if (input_mq == -1) {
        pthread_mutex_lock(args->stderr_mutex);
        perror("BlockThread");
        pthread_mutex_unlock(args->stderr_mutex);
        pthread_exit(NULL);
    }

    mq_sem = sem_open(args->queue_sem_name, O_RDONLY | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                      0);

    if (mq_sem == SEM_FAILED) {
        pthread_mutex_lock(args->stderr_mutex);
        perror("BlockThread");
        pthread_mutex_unlock(args->stderr_mutex);
        mq_close(input_mq);
        pthread_exit(NULL);
    }

    buffer_sem = sem_open(args->buffer_sem_name, O_RDONLY | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                      0);

    if (buffer_sem == SEM_FAILED) {
        pthread_mutex_lock(args->stderr_mutex);
        perror("BlockThread");
        pthread_mutex_unlock(args->stderr_mutex);
        mq_close(input_mq);
        sem_close(mq_sem);
        pthread_exit(NULL);
    }

    /* TODO Set up cleanup routine */
    cleanup_arg_t cleanupArg = {
            .mq_name = args->mq_name,
            .buffer_sem_name = args->buffer_sem_name,
            .queue_sem_name = args->queue_sem_name,
            .mq_handle = input_mq,
            .buffer_sem_handle = buffer_sem,
            .queue_sem_handle = mq_sem
    };

    pthread_cleanup_push(BlockThreadCleanup, &cleanupArg);

    while (1) {
        sem_wait(mq_sem);
        char msgbuf[2];
        char *block_buffer = args->buffer;

        if (mq_receive(input_mq, msgbuf, 2, NULL) < 0) {
            pthread_mutex_lock(args->stderr_mutex);
            perror("BlockThread");
            pthread_mutex_unlock(args->stderr_mutex);
            mq_close(input_mq);
            pthread_exit(NULL);
        }

        char input = msgbuf[0];

        if (input == ' ') {
            /* Terminate the buffer string */
            pthread_mutex_lock(args->buffer_mutex);
            block_buffer[i] = '\0';
            pthread_mutex_unlock(args->buffer_mutex);
            i = 0;

            /* Buffer is ready to read */
            sem_post(buffer_sem);
        } else {
            pthread_mutex_lock(args->buffer_mutex);
            block_buffer[i] = input;
            pthread_mutex_unlock(args->buffer_mutex);
            i++;
        }
        /* TODO handle buffer being full */
    }

    pthread_cleanup_pop(1);

    return NULL;
}
