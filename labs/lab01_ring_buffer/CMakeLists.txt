# Lab 1 - 环形缓冲区

# 被测库：你的实现在 src/ring_buffer.c
add_library(lab01_ring_buffer STATIC
    src/ring_buffer.c)

target_include_directories(lab01_ring_buffer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 测试可执行文件
add_executable(test_lab01_ring_buffer
    test/test_ring_buffer.c)

target_link_libraries(test_lab01_ring_buffer PRIVATE
    lab01_ring_buffer
    labs_common)   # 提供 common/test_framework.h 的 include 路径

# 注册到 CTest
add_test(NAME lab01_ring_buffer
         COMMAND test_lab01_ring_buffer)
