# Lab 4 - 哈希表

add_library(lab04_hash_table STATIC
    src/hash_table.c)

target_include_directories(lab04_hash_table PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_lab04_hash_table
    test/test_hash_table.c)

target_link_libraries(test_lab04_hash_table PRIVATE
    lab04_hash_table
    labs_common)

add_test(NAME lab04_hash_table
         COMMAND test_lab04_hash_table)
