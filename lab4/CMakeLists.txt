cmake_minimum_required(VERSION 3.10)
project(lab4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executables
add_executable(temp_sensor src/temp_sensor.cpp)
add_executable(temp_monitor src/temp_monitor.cpp)

# Platform specific libraries
if(WIN32)
    target_link_libraries(temp_sensor PRIVATE setupapi)
    target_link_libraries(temp_monitor PRIVATE setupapi)
elseif(UNIX)
    target_link_libraries(temp_sensor PRIVATE pthread)
    target_link_libraries(temp_monitor PRIVATE pthread)
endif() 