cmake_minimum_required(VERSION 3.0)
project(Tracer)
set (CMAKE_CXX_STANDARD 14)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(HEADER_FILES include/Tracer.h)

add_library(tracer INTERFACE)
target_include_directories(tracer INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:tracer/include>
    )
add_subdirectory(tests)


install(TARGETS tracer
        EXPORT tracer
        LIBRARY DESTINATION tracer/lib)

install(FILES ${HEADER_FILES} DESTINATION tracer/include)

install(EXPORT tracer
        NAMESPACE tracer:: DESTINATION tracer)
install(FILES tracerConfig.cmake
        DESTINATION tracer)


