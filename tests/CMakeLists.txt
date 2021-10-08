set(SOURCES
        main.cu
        accel_tests.cu
        common_tests.cu
        core_tests.cu
        geometry_tests.cu
        renderers_tests.cu
        sampling_tests.cu
        shape_tests.cu
        )

add_executable(helios_tests ${SOURCES})
add_dependencies(helios_tests hermes)

set_target_properties(helios_tests PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(helios_tests PUBLIC ${CATCH2_INCLUDES} ${HERMES_INCLUDES})

add_definitions(
        -DENABLE_CUDA=1
        -D__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__=1)
target_compile_options(helios_tests PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
        --generate-line-info
        --use_fast_math
        #-arch=sm_50
        -Xcompiler -pg
        --relocatable-device-code=true
        #    -–extended-lambda
        >)

set_target_properties(helios_tests PROPERTIES
        LINKER_LANGUAGE CUDA
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
        CUDA_STANDARD 17
        CMAKE_CUDA_STANDARD_REQUIRED ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        )

if (UNIX)

    target_link_libraries(helios_tests ${HERMES_LIBRARIES} helios gcov)

    add_custom_target(test_helios
            COMMAND lcov --directory . --zerocounters
            COMMAND helios_tests DEPENDS helios_tests
            COMMAND lcov --directory . --capture -output-file cov.info
            COMMAND lcov --remove cov.info '/usr/*' --output-file cov.info
            COMMAND lcov --remove cov.info '*/ext/*' --output-file cov.info
            COMMAND lcov --remove cov.info '*/tests/*' --output-file cov.info
            COMMAND genhtml cov.info --output-directory out
            )

else (UNIX)

    target_link_libraries(helios_tests helios)

endif (UNIX)
