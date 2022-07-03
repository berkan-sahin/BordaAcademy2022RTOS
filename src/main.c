#include <stdio.h>
#include "ListenerThread.h"


int main() {
    
    ListenerThread_arg_t lt_args = {
            .block_sem_name = "/block_sem",
            .stream_sem_name = "/stream_sem",
            .stream_mq_name = "/stream_mq",
            .block_mq_name = "/block_mq_name",
            .stderr_mutex = &stderr_mutex,
            .stdin_mutex = &stdin_mutex,
            .queue_attrs = &queue_attrs

    };

    pthread_create(&listener, NULL, ListenerThread, &lt_args);
    pthread_join(listener, NULL);
    return 0;
}
