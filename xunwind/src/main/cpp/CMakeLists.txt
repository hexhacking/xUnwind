cmake_minimum_required(VERSION 3.10.2)

add_compile_options(
        -std=c11
        -Weverything
        -Werror)

find_package(xdl REQUIRED CONFIG)

file(GLOB XUNWIND_SRC
        *.c)

add_library(xunwind SHARED
        ${XUNWIND_SRC})

target_include_directories(xunwind PUBLIC
        include
        .)

target_link_libraries(xunwind
        xdl::xdl
        log)

if(USEASAN)

target_compile_options(xunwind PUBLIC
        -fsanitize=address
        -fno-omit-frame-pointer)

set_target_properties(xunwind PROPERTIES
        LINK_FLAGS " \
        -fsanitize=address")

else()

target_compile_options(xunwind PUBLIC
        -Oz
        -flto
        -ffunction-sections
        -fdata-sections)

set_target_properties(xunwind PROPERTIES
        LINK_FLAGS " \
        -O3 \
        -flto \
        -Wl,--exclude-libs,ALL \
        -Wl,--gc-sections \
        -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/xunwind.map.txt")

#target_compile_options(xunwind PUBLIC -O0 -g)
#set_target_properties(xunwind PROPERTIES LINK_FLAGS "-O0")

endif()
