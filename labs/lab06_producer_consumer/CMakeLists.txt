# Lab 6 - pthread 生产者-消费者

find_package(Threads REQUIRED)

add_library(lab06_producer_consumer STATIC
    src/sync_buffer.c)

target_include_directories(lab06_producer_consumer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(lab06_producer_consumer PUBLIC
    Threads::Threads)

add_executable(test_lab06_producer_consumer
    test/test_sync_buffer.c)

target_link_libraries(test_lab06_producer_consumer PRIVATE
    lab06_producer_consumer
    labs_common
    Threads::Threads)

add_test(NAME lab06_producer_consumer
         COMMAND test_lab06_producer_consumer)
