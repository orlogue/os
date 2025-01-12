add_library(process_lib
    src/process_lib_unix.cpp
    src/process_lib_win.cpp
)

target_include_directories(process_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS process_lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES include/process_lib.h
    DESTINATION include
) 