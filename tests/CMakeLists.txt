set(TEST_SRC
    main.cpp
    )
add_executable(tracer_unit_tests ${TEST_SRC} ../include/Tracer.h)
target_include_directories(tracer_unit_tests  PUBLIC ../include )

target_link_libraries(tracer_unit_tests tracer)



