cmake_minimum_required(VERSION 3.22.1)
project(sample)

find_package(xunwind REQUIRED CONFIG)

add_library(sample SHARED sample.c)
target_compile_features(sample PUBLIC c_std_17)
target_compile_options(sample PUBLIC -std=c17 -Weverything -Werror)
if(${ANDROID_ABI} STREQUAL "arm64-v8a")
    target_compile_options(sample PUBLIC -mbranch-protection=standard)
endif()
target_link_libraries(sample xunwind::xunwind log)
