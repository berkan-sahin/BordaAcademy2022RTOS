#include <stdio.h>
#include <mqueue.h>
#include <semaphore.h>
#include <pthread.h>
#include "ListenerThread.h"
#include "StreamThread.h"

/*
typedef struct {
    const char *sem_name;
    const char *mq_name;
    struct mq_attr *queue_attrs;
    pthread_mutex_t *stdout_mutex;
    pthread_mutex_t *stderr_mutex;
    pthread_t *listener_handle;
    pthread_t *stream_handle;
} DummyThread_arg_t;


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
        /* Block until new message arrives
        sem_wait(mq_sem);
        char msgbuf[2];

        if (mq_receive(input_mq, msgbuf, 2, NULL) < 0) {
            pthread_mutex_lock(args->stderr_mutex);
            perror("DummyThread");
            pthread_mutex_unlock(args->stderr_mutex);
            mq_close(input_mq);
            pthread_exit(NULL);
        }

        /* TODO Quit if the message is "q"

        if (msgbuf[0] == 'q') {
            pthread_mutex_lock(args->stdout_mutex);
            printf("Received \"q\", quitting...");
            pthread_mutex_unlock(args->stdout_mutex);

            pthread_cancel(*args->listener_handle);
            pthread_cancel(*args->stream_handle);
            pthread_exit(NULL);
        }

    }


    return NULL;
}
*/

int main() {

    /* I/O mutexes */
    pthread_mutex_t stderr_mutex, stdin_mutex, stdout_mutex;
    pthread_t listener, stream_thread, block_thread;
    /* Shared buffers */
    char block_buffer[512];
    const int BUFFER_SIZE = 512;
    pthread_mutex_t block_buf_mutex, command_buf_mutex;
    const char *BLOCK_SEM_NAME = "/block_input";
    const char *BLOCK_MQ_NAME = "/blockmq";
    const char *BLOCKBUF_SEM_NAME = "/blockbuf_ready";
    pthread_mutex_init(&stderr_mutex, NULL);
    pthread_mutex_init(&stdin_mutex, NULL);
    pthread_mutex_init(&stdout_mutex, NULL);
    pthread_mutex_init(&block_buf_mutex, NULL);

    queue_attrs.mq_msgsize = 2;
    queue_attrs.mq_maxmsg = 8;

    ListenerThread_arg_t lt_args = {
            .block_sem_name = BLOCK_SEM_NAME,
            .stream_sem_name = "/streamsem",
            .stream_mq_name = "/streammq",
            .block_mq_name = BLOCK_MQ_NAME,
            .stderr_mutex = &stderr_mutex,
            .stdin_mutex = &stdin_mutex,
            .queue_attrs = &queue_attrs
    };

    StreamThread_arg_t stream_thread_args = {
        .mq_name = "/streammq",
        .sem_name = "/streamsem",
        .queue_attrs = &queue_attrs,
        .stdout_mutex = &stdout_mutex,
        .stderr_mutex = &stderr_mutex
    };

    BlockThread_arg_t block_thread_args = {
            .stderr_mutex = &stderr_mutex,
            .buffer_sem_name = BLOCKBUF_SEM_NAME,
            .buffer = block_buffer,
            .BUF_MAXSIZE = BUFFER_SIZE,
            .buffer_mutex = &block_buf_mutex
    };

    pthread_create(&listener, NULL, ListenerThread, &lt_args);
    pthread_create(&block_thread, NULL, BlockThread, &block_thread_args);
    pthread_create(&stream_thread, NULL, StreamThread, &stream_thread_args);

    /* Wait for the threads to finish */
    pthread_join(listener, NULL);
    pthread_join(stream_thread, NULL);
    pthread_join(block_thread, NULL);

    return 0;
}
