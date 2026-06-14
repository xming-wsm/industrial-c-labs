# Lab 5 - 表驱动有限状态机

add_library(lab05_fsm STATIC
    src/fsm.c)

target_include_directories(lab05_fsm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_lab05_fsm
    test/test_fsm.c)

target_link_libraries(test_lab05_fsm PRIVATE
    lab05_fsm
    labs_common)

add_test(NAME lab05_fsm
         COMMAND test_lab05_fsm)
