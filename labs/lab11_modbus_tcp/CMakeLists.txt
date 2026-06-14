# Lab 11 - Modbus TCP 从站模拟

add_library(lab11_modbus_tcp STATIC
    src/modbus_slave.c)

target_include_directories(lab11_modbus_tcp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_lab11_modbus_tcp
    test/test_modbus_slave.c)

target_link_libraries(test_lab11_modbus_tcp PRIVATE
    lab11_modbus_tcp
    labs_common)

add_test(NAME lab11_modbus_tcp
         COMMAND test_lab11_modbus_tcp)
