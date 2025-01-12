cmake_minimum_required(VERSION 3.10)
project(process_lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

add_subdirectory(lib/process_lib)

add_executable(process_test
    test/test.cpp
)

target_link_libraries(process_test PRIVATE process_lib) 

add_custom_command(TARGET process_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:process_lib>
        $<TARGET_FILE_DIR:process_test>
)