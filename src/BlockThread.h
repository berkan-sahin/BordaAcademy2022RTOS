//
// Created by bsahin on 5/07/22.
//

#ifndef BORDAACADEMY2022RTOS_BLOCKTHREAD_H
#include <mqueue.h>
#include <pthread.h>

typedef struct {
    const char *mq_name;
    const char *queue_sem_name;
    struct mq_attr *queue_attrs;
    pthread_mutex_t *stderr_mutex;

    const char *buffer_sem_name;
    char *buffer;
    int BUF_MAXSIZE;
    pthread_mutex_t *buffer_mutex;
} BlockThread_arg_t;

void *BlockThread(void *arg);

#define BORDAACADEMY2022RTOS_BLOCKTHREAD_H

#endif //BORDAACADEMY2022RTOS_BLOCKTHREAD_H
