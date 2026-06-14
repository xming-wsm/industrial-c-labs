# Lab 7 - 线程池 + 任务队列

find_package(Threads REQUIRED)

add_library(lab07_thread_pool STATIC
    src/thread_pool.c)

target_include_directories(lab07_thread_pool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(lab07_thread_pool PUBLIC
    Threads::Threads)

add_executable(test_lab07_thread_pool
    test/test_thread_pool.c)

target_link_libraries(test_lab07_thread_pool PRIVATE
    lab07_thread_pool
    labs_common
    Threads::Threads)

add_test(NAME lab07_thread_pool
         COMMAND test_lab07_thread_pool)
