# Lab 10 - 协议帧解析 + CRC16

add_library(lab10_frame_parser STATIC
    src/frame_parser.c)

target_include_directories(lab10_frame_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_lab10_frame_parser
    test/test_frame_parser.c)

target_link_libraries(test_lab10_frame_parser PRIVATE
    lab10_frame_parser
    labs_common)

add_test(NAME lab10_frame_parser
         COMMAND test_lab10_frame_parser)
