# Lab 3 - 链表 + 静态内存池

add_library(lab03_linked_list_pool STATIC
    src/alarm_list.c)

target_include_directories(lab03_linked_list_pool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_lab03_linked_list_pool
    test/test_alarm_list.c)

target_link_libraries(test_lab03_linked_list_pool PRIVATE
    lab03_linked_list_pool
    labs_common)

add_test(NAME lab03_linked_list_pool
         COMMAND test_lab03_linked_list_pool)
