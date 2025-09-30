cmake_minimum_required(VERSION 2.8.12)

# 如果作为子目录使用，可以不写 project()
# project(serial)

set(serial_SRCS
    src/serial.cc
    include/serial/serial.h
    include/serial/v8stdint.h
)

if(APPLE)
    # macOS
    list(APPEND serial_SRCS
        src/impl/unix.cc
        src/impl/list_ports/list_ports_osx.cc
    )
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)

elseif(UNIX)
    # Linux
    list(APPEND serial_SRCS
        src/impl/unix.cc
        src/impl/list_ports/list_ports_linux.cc
    )

else()
    # Windows
    list(APPEND serial_SRCS
        src/impl/win.cc
        src/impl/list_ports/list_ports_win.cc
    )
endif()

add_library(serial ${serial_SRCS})

target_include_directories(serial PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(APPLE)
    target_link_libraries(serial PRIVATE ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
    target_link_libraries(serial PRIVATE pthread rt)
else()
    target_link_libraries(serial PRIVATE setupapi)
endif()
