# Lab 9 - TCP/UDP socket 上位机通信

find_package(Threads REQUIRED)

add_library(lab09_socket_comm STATIC
    src/socket_comm.c)

target_include_directories(lab09_socket_comm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_lab09_socket_comm
    test/test_socket_comm.c)

target_link_libraries(test_lab09_socket_comm PRIVATE
    lab09_socket_comm
    labs_common
    Threads::Threads)

add_test(NAME lab09_socket_comm
         COMMAND test_lab09_socket_comm)
