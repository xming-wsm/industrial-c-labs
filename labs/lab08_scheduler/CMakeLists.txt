# Lab 8 - 周期任务调度 / 定时器

add_library(lab08_scheduler STATIC
    src/scheduler.c)

target_include_directories(lab08_scheduler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_lab08_scheduler
    test/test_scheduler.c)

target_link_libraries(test_lab08_scheduler PRIVATE
    lab08_scheduler
    labs_common)

add_test(NAME lab08_scheduler
         COMMAND test_lab08_scheduler)
