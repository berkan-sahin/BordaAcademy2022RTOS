cmake_minimum_required(VERSION 3.16)
project(BordaAcademy2022RTOS C)

set(CMAKE_C_STANDARD 99)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(BordaAcademy2022RTOS main.c)
target_compile_options(BordaAcademy2022RTOS PUBLIC -pedantic -Wall -Wextra)
target_link_libraries(BordaAcademy2022RTOS PRIVATE Threads::Threads)
target_link_options(BordaAcademy2022RTOS PUBLIC -pthread -lrt)