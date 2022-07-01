#include <stdio.h>
#include "ListenerThread.h"


int main() {
    
    ListenerThread_arg_t lt_args = {
            .block_sem_name = "/block_sem",
            .stream_sem_name = "/stream_sem",
            .stream_mq_name = "/stream_mq",
            .block_mq_name = "/block_mq_name",

    };

    printf("Hello, World!\n");
    return 0;
}
