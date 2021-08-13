# extracted from https://github.com/SaschaWillems/Vulkan/blob/master/examples/CMakeLists.txt
function(buildExample EXAMPLE_NAME)
    set(EXAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME})
    message(STATUS "Generating project file for example in ${EXAMPLE_FOLDER}")
    # Main
    SET(MAIN_CPP ${EXAMPLE_NAME}.cu)
    set_source_files_properties(${MAIN_CPP} PROPERTIES LANGUAGE CUDA)
    # Add shaders
    set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME}/shaders")
    file(GLOB SHADERS
            "${SHADER_DIR}/*.vert"
            "${SHADER_DIR}/*.frag"
            "${SHADER_DIR}/*.comp"
            "${SHADER_DIR}/*.geom"
            "${SHADER_DIR}/*.tesc"
            "${SHADER_DIR}/*.tese"
            "${SHADER_DIR}/*.mesh"
            "${SHADER_DIR}/*.task"
            "${SHADER_DIR}/*.rgen"
            "${SHADER_DIR}/*.rchit"
            "${SHADER_DIR}/*.rmiss"
            )
    add_executable(${EXAMPLE_NAME} ${MAIN_CPP} ${SHADERS})
    add_dependencies(${EXAMPLE_NAME} helios circe)
    target_compile_definitions( ${EXAMPLE_NAME} PUBLIC
            -DENABLE_CUDA=1
            -D__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__=1)
    target_link_libraries(${EXAMPLE_NAME}
            helios
            ${CIRCE_LIBRARIES}
            ${GLFW_LIBRARIES}
            ${HERMES_LIBRARIES}
            )
    target_include_directories(${EXAMPLE_NAME} PUBLIC
            ${HELIOS_SOURCE_DIR}
            ${HERMES_INCLUDES}
            ${CIRCE_INCLUDES}
            ${STB_INCLUDES}
            ${GLFW_INCLUDES}
            ${VULKAN_INCLUDES}
            )
    target_compile_definitions(${EXAMPLE_NAME} PUBLIC
            -DSHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
            -DMODELS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/models"
            -DTEXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/textures"
            -D__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__=1
            )
    target_compile_options(${EXAMPLE_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
            --generate-line-info
            --use_fast_math
            #-arch=sm_50
            -Xcompiler -pg
            --relocatable-device-code=true
            #    -–extended-lambda
            >)
    set_target_properties(${EXAMPLE_NAME} PROPERTIES
            LINKER_LANGUAGE CUDA
            CMAKE_CUDA_SEPARABLE_COMPILATION ON
            CUDA_RESOLVE_DEVICE_SYMBOLS ON
            POSITION_INDEPENDENT_CODE ON
            CUDA_STANDARD 17
            CUDA_STANDARD_REQUIRED ON
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            )
endfunction(buildExample)

set(EXAMPLES
        camera_samples_vis
        )

foreach (EXAMPLE ${EXAMPLES})
    buildExample(${EXAMPLE})
endforeach (EXAMPLE)
