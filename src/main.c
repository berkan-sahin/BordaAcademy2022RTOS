#include <stdio.h>
#include <mqueue.h>
#include <semaphore.h>
#include <pthread.h>
#include "ListenerThread.h"


typedef struct {
    const char *sem_name;
    const char *mq_name;
    const char *thread_name;
    struct mq_attr *queue_attrs;
    pthread_mutex_t *stdout_mutex;
    pthread_mutex_t *stderr_mutex;
} DummyThread_arg_t;

/**
 * A dummy thread to test ListenerThread. Prints each message to stdout.
 * @param arg
 * @return NULL
 */
void *DummyThread(void *arg) {
    DummyThread_arg_t *args = (DummyThread_arg_t *) arg;

    mqd_t input_mq;
    sem_t *mq_sem;

    input_mq = mq_open(args->mq_name, O_CREAT | O_RDONLY,
                          S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                          args->queue_attrs);

    if (input_mq == -1) {
        pthread_mutex_lock(args->stderr_mutex);
        perror("DummyThread");
        pthread_mutex_unlock(args->stderr_mutex);
        pthread_exit(NULL);
    }

    mq_sem = sem_open(args->sem_name, O_RDONLY | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP,
                      0);

    if (mq_sem == SEM_FAILED) {
        pthread_mutex_lock(args->stderr_mutex);
        perror("DummyThread");
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
            perror("DummyThread");
            pthread_mutex_unlock(args->stderr_mutex);
            mq_close(input_mq);
            pthread_exit(NULL);
        }

        pthread_mutex_lock(args->stdout_mutex);
        printf("%s received %s\n", args->thread_name, msgbuf);
        pthread_mutex_unlock(args->stdout_mutex);

        /* TODO Quit if the message is "q" */
    }


    return NULL;
}

int main() {

    /* I/O mutexes */
    pthread_mutex_t stderr_mutex, stdin_mutex, stdout_mutex;
    pthread_t listener, stream_thread, block_thread;
    struct mq_attr queue_attrs;
    pthread_mutex_init(&stderr_mutex, NULL);
    pthread_mutex_init(&stdin_mutex, NULL);
    pthread_mutex_init(&stdout_mutex, NULL);

    queue_attrs.mq_msgsize = 2;
    queue_attrs.mq_maxmsg = 8;

    ListenerThread_arg_t lt_args = {
            .block_sem_name = "/blocksem",
            .stream_sem_name = "/streamsem",
            .stream_mq_name = "/streammq",
            .block_mq_name = "/blockmq",
            .stderr_mutex = &stderr_mutex,
            .stdin_mutex = &stdin_mutex,
            .queue_attrs = &queue_attrs
    };

    DummyThread_arg_t stream_thread_args = {
            .stderr_mutex = &stderr_mutex,
            .stdout_mutex = &stdout_mutex,
            .thread_name = "Dummy stream thread",
            .mq_name = "/streammq",
            .sem_name = "/streamsem",
            .queue_attrs = &queue_attrs
    };

    DummyThread_arg_t block_thread_args = {
            .stderr_mutex = &stderr_mutex,
            .stdout_mutex = &stdout_mutex,
            .thread_name = "Dummy block thread",
            .mq_name = "/blockmq",
            .sem_name = "/blocksem",
            .queue_attrs = &queue_attrs
    };

    pthread_create(&listener, NULL, ListenerThread, &lt_args);
    pthread_create(&block_thread, NULL, DummyThread, &block_thread_args);
    pthread_create(&stream_thread, NULL, DummyThread, &stream_thread_args);

    /* Wait for the threads to finish */
    pthread_join(listener, NULL);
    pthread_join(stream_thread, NULL);
    pthread_join(block_thread, NULL);

    return 0;
}
