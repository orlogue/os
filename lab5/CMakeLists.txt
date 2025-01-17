cmake_minimum_required(VERSION 3.15)
project(temperature_monitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
endif()

find_package(Boost REQUIRED COMPONENTS system thread filesystem json)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

set(MONITOR_SOURCES
    src/temp_monitor.cpp
    src/serial_port_unix.cpp
    src/serial_port_win.cpp
    src/http_server.cpp
    src/http_session.cpp
    src/db_manager.cpp
    src/api_handler.cpp
)

set(SENSOR_SOURCES
    src/temp_sensor.cpp
    src/serial_port_unix.cpp
    src/serial_port_win.cpp
)

add_executable(temperature_monitor ${MONITOR_SOURCES})
add_executable(temp_sensor ${SENSOR_SOURCES})

foreach(TARGET temperature_monitor temp_sensor)
    target_include_directories(${TARGET} PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        ${SQLite3_INCLUDE_DIRS}
    )
endforeach()

target_link_libraries(temperature_monitor PRIVATE 
    Boost::system
    Boost::thread
    Boost::filesystem
    Boost::json
    OpenSSL::SSL
    OpenSSL::Crypto
    SQLite::SQLite3
)

target_link_libraries(temp_sensor PRIVATE
    Boost::system
)

if(WIN32)
    target_link_libraries(temperature_monitor PRIVATE setupapi)
    target_link_libraries(temp_sensor PRIVATE setupapi)
else()
    target_link_libraries(temperature_monitor PRIVATE pthread)
    target_link_libraries(temp_sensor PRIVATE pthread)
endif()

add_custom_command(TARGET temperature_monitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/frontend/build
    $<TARGET_FILE_DIR:temperature_monitor>/public
)

message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "SQLite3 include dirs: ${SQLite3_INCLUDE_DIRS}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}") 