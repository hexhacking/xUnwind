cmake_minimum_required(VERSION 3.18.1)
project(sample)

if(DEPENDENCY_ON_LOCAL_LIBRARY)
    set(XUNWIND_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../xunwind)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(XUNWIND_LIB_BASE ${XUNWIND_BASE}/build/intermediates/merged_native_libs/debug)
    else()
        set(XUNWIND_LIB_BASE ${XUNWIND_BASE}/build/intermediates/stripped_native_libs/release)
    endif()
    add_library(xunwind::xunwind SHARED IMPORTED)
    set_target_properties(xunwind::xunwind PROPERTIES
            IMPORTED_LOCATION ${XUNWIND_LIB_BASE}/out/lib/${ANDROID_ABI}/libxunwind.so
            INTERFACE_INCLUDE_DIRECTORIES ${XUNWIND_BASE}/src/main/cpp/include)
else()
    find_package(xunwind REQUIRED CONFIG)
endif()

add_library(sample SHARED sample.c)
target_compile_features(sample PUBLIC c_std_11)
target_compile_options(sample PUBLIC -Weverything -Werror)
target_link_libraries(sample xunwind::xunwind log)
