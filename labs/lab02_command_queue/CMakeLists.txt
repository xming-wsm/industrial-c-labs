# Lab 2 - 命令队列 / 双端队列

# 被测库：你的实现在 src/command_queue.c
add_library(lab02_command_queue STATIC
    src/command_queue.c)

target_include_directories(lab02_command_queue PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 测试可执行文件
add_executable(test_lab02_command_queue
    test/test_command_queue.c)

target_link_libraries(test_lab02_command_queue PRIVATE
    lab02_command_queue
    labs_common)

# 注册到 CTest
add_test(NAME lab02_command_queue
         COMMAND test_lab02_command_queue)
