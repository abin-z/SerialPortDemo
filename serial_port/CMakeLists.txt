# 添加第三方依赖
add_subdirectory(3rd/serial)

# 生成静态库
add_library(serialport STATIC src/serialport.cpp)

# 链接依赖, serial也暴露给使用serialport的用户
target_link_libraries(serialport PUBLIC serial)

# 头文件对外可见
target_include_directories(serialport PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
