cmake_minimum_required(VERSION 3.10)
project(lab4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SENSOR_SOURCES src/temp_sensor.cpp)
set(MONITOR_SOURCES src/temp_monitor.cpp)

if(WIN32)
    list(APPEND SENSOR_SOURCES src/serial_port_win.cpp)
    list(APPEND MONITOR_SOURCES src/serial_port_win.cpp)
else()
    list(APPEND SENSOR_SOURCES src/serial_port_unix.cpp)
    list(APPEND MONITOR_SOURCES src/serial_port_unix.cpp)
endif()

add_executable(temp_sensor ${SENSOR_SOURCES})
add_executable(temp_monitor ${MONITOR_SOURCES})

if(WIN32)
    target_link_libraries(temp_sensor PRIVATE setupapi)
    target_link_libraries(temp_monitor PRIVATE setupapi)
else()
    target_link_libraries(temp_sensor PRIVATE pthread)
    target_link_libraries(temp_monitor PRIVATE pthread)
endif() 