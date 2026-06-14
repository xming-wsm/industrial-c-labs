cmake_minimum_required(VERSION 3.16)

project(industrial_c_labs
    VERSION 0.1.0
    DESCRIPTION "Industrial C Labs - 注塑机控制系统实验课"
    LANGUAGES C)

# ---- 全局编译选项 ----
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 默认导出 compile_commands.json，便于编辑器/clangd 索引
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 没有显式指定构建类型时，默认 Debug，方便调试与断言
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# 统一打开常用告警；把告警当作学习反馈，不强制 -Werror（新手友好）
add_compile_options(-Wall -Wextra -Wshadow)

# ---- 公共头文件（单测框架等） ----
add_library(labs_common INTERFACE)
target_include_directories(labs_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/common)

# ---- 测试 ----
enable_testing()

# ---- Labs（逐关解锁；过关后取消下一行注释即可解锁） ----
# 说明：Lab 2~11 的脚手架（指导书 + 头文件 + 实现桩 + 测试）均已就绪，
#       为保持"实现 → 测试 → 解锁"的学习闭环，默认只启用当前关，
#       过一关就把下一行的注释去掉。
add_subdirectory(labs/lab01_ring_buffer)
# add_subdirectory(labs/lab02_command_queue)      # Track A：命令队列 / 双端队列
# add_subdirectory(labs/lab03_linked_list_pool)   # Track A：链表 + 静态内存池
# add_subdirectory(labs/lab04_hash_table)         # Track A：哈希表
# add_subdirectory(labs/lab05_fsm)                # Track A：表驱动状态机
# add_subdirectory(labs/lab06_producer_consumer)  # Track B：pthread 生产者-消费者
# add_subdirectory(labs/lab07_thread_pool)        # Track B：线程池 + 任务队列
# add_subdirectory(labs/lab08_scheduler)          # Track B：周期任务调度 / 定时器
# add_subdirectory(labs/lab09_socket_comm)        # Track C：TCP/UDP socket
# add_subdirectory(labs/lab10_frame_parser)       # Track C：协议帧解析 + CRC16
# add_subdirectory(labs/lab11_modbus_tcp)         # Track C：Modbus TCP 从站
