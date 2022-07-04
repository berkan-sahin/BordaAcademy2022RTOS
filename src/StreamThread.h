//
// Created by bsahin on 4/07/22.
//

#ifndef BORDAACADEMY2022RTOS_STREAMTHREAD_H
#define BORDAACADEMY2022RTOS_STREAMTHREAD_H
#include <mqueue.h>
#include <pthread.h>

typedef struct {
    const char* mq_name;
    const char* sem_name;
    struct mq_attr *queue_attrs;
    pthread_mutex_t *stdout_mutex;
    pthread_mutex_t *stderr_mutex;
} StreamThread_arg_t;

void *StreamThread(void *arg);

#endif //BORDAACADEMY2022RTOS_STREAMTHREAD_H
