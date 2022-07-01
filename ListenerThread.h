//
// Created by ayupa on 1/07/2022.
//

#ifndef BORDAACADEMY2022RTOS_LISTENERTHREAD_H
#define BORDAACADEMY2022RTOS_LISTENERTHREAD_H
#include <pthread.h>

typedef struct  {
    const char* block_mq_name;
    const char* stream_mq_name;
    const char* block_sem_name;
    const char* stream_sem_name;
    pthread_mutex_t *stderr_mutex;
    pthread_mutex_t *stdin_mutex;
} ListenerThread_arg_t;


void *ListenerThread(void *arg);


#endif //BORDAACADEMY2022RTOS_LISTENERTHREAD_H
