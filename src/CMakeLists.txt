set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_library(LIBRT rt)

add_executable(BordaAcademy2022RTOS main.c ListenerThread.c ListenerThread.h StreamThread.c StreamThread.h BlockThread.c BlockThread.h)
target_compile_options(BordaAcademy2022RTOS PUBLIC -pedantic -Wall -Wextra -lrt)
target_link_libraries(BordaAcademy2022RTOS PRIVATE Threads::Threads rt)
target_link_options(BordaAcademy2022RTOS PUBLIC -pthread -lrt)