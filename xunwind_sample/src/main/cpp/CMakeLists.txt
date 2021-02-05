cmake_minimum_required(VERSION 3.10.2)

add_compile_options(
        -std=c11
        -Weverything
        -Werror)

# import xunwind by local-project
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_DIRNAME)
set(XUNWIND_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../xunwind)
add_library(xunwind::xunwind SHARED IMPORTED)
set_target_properties(xunwind::xunwind PROPERTIES
        IMPORTED_LOCATION ${XUNWIND_BASE}/build/intermediates/stripped_native_libs/${BUILD_TYPE_DIRNAME}/out/lib/${ANDROID_ABI}/libxunwind.so
        INTERFACE_INCLUDE_DIRECTORIES ${XUNWIND_BASE}/src/main/cpp/include
        )

# import xunwind by maven
#find_package(xunwind REQUIRED CONFIG)

add_library(sample SHARED
        sample.c)

target_link_libraries(sample
        xunwind::xunwind
        log)
