cmake_minimum_required(VERSION 3.18.1)
project(xunwind)

find_package(xdl REQUIRED CONFIG)

file(GLOB XUNWIND_SRC *.c)
add_library(xunwind SHARED ${XUNWIND_SRC})
target_compile_features(xunwind PUBLIC c_std_11)
target_compile_options(xunwind PUBLIC -Weverything -Werror)
target_include_directories(xunwind PUBLIC include .)
target_link_libraries(xunwind xdl::xdl log)

if(USEASAN)
target_compile_options(xunwind PUBLIC -fsanitize=address -fno-omit-frame-pointer)
target_link_options(xunwind PUBLIC -fsanitize=address)
else()
target_compile_options(xunwind PUBLIC -Oz -flto -faddrsig -ffunction-sections -fdata-sections)
target_link_options(xunwind PUBLIC -Oz -flto -Wl,--icf=all -Wl,-mllvm,--enable-machine-outliner=always -Wl,--exclude-libs,ALL -Wl,--gc-sections -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/xunwind.map.txt)
endif()
