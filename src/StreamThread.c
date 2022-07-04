//
// Created by bsahin on 4/07/22.
//

#include "StreamThread.h"
#include <semaphore.h>
#include <stdio.h>

void *StreamThread(void *arg) {
    /* The table used for stream encoding.
     * The values are arbitrary */
    const int table[] = {1, 2, 3, 4, 5, 6, 2, 1, 2};
    const int table_size = 8;
    int i = 0;


    StreamThread_arg_t *args = (StreamThread_arg_t *) arg;

    mqd_t input_mq;
    sem_t *mq_sem;

    input_mq = mq_open(args->mq_name, O_CREAT | O_RDONLY,
                       S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                       args->queue_attrs);

    if (input_mq == -1) {
        pthread_mutex_lock(args->stderr_mutex);
        perror("StreamThread");
        pthread_mutex_unlock(args->stderr_mutex);
        pthread_exit(NULL);
    }

    mq_sem = sem_open(args->sem_name, O_RDONLY | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                      0);

    if (mq_sem == SEM_FAILED) {
        pthread_mutex_lock(args->stderr_mutex);
        perror("StreamThread");
        pthread_mutex_unlock(args->stderr_mutex);
        mq_close(input_mq);
        pthread_exit(NULL);
    }

    while (1) {
        /* Block until new message arrives */
        sem_wait(mq_sem);
        char msgbuf[2];

        if (mq_receive(input_mq, msgbuf, 2, NULL) < 0) {
            pthread_mutex_lock(args->stderr_mutex);
            perror("StreamThread");
            pthread_mutex_unlock(args->stderr_mutex);
            mq_close(input_mq);
            pthread_exit(NULL);
        }

        char output = msgbuf[0] + table[i];

        i++;
        if (i >= table_size)
            i = 0;

        pthread_mutex_lock(args->stdout_mutex);
        printf("%c\n", output);
        pthread_mutex_unlock(args->stdout_mutex);


    }

    return NULL;
}
