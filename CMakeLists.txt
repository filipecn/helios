# avoid policy warning message
if (POLICY CMP0104)
    cmake_policy(SET CMP0104 NEW)
endif (POLICY CMP0104)
# main project
set(CMAKE_CUDA_COMPILER /usr/local/cuda-11.0/bin/nvcc)
project(HELIOS C CXX CUDA)
cmake_minimum_required(VERSION 3.18)
cmake_policy(VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)
##########################################
##               options                ##
##########################################
option(BUILD_ALL "build all libraries" OFF)
option(BUILD_TESTS "build library unit tests" OFF)
option(BUILD_VIS "build visual app" OFF)
option(BUILD_EXAMPLES "build library examples" OFF)
option(BUILD_SHARED "build shared library" OFF)
option(BUILD_DOCS "build library documentation" OFF)
set(INSTALL_PATH ${BUILD_ROOT} CACHE STRING "include and lib folders path")
# external libs
set(HERMES_INCLUDE_PATH "" CACHE STRING "hermes include path")
set(HERMES_LIB_PATH "" CACHE STRING "hermes lib path")
set(CIRCE_INCLUDE_PATH "" CACHE STRING "hermes include path")
set(CIRCE_LIB_PATH "" CACHE STRING "hermes lib path")
##########################################
##            compilation               ##
##########################################
option(BUILD_ALL "build all libraries" OFF)
# cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
# check for specific machine/compiler options.
include(ConfigCompilerFlags)
#find_package(CUDA REQUIRED)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
##########################################
##           dependencies               ##
##########################################
# include external libraries path
include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext")
include(hermes)
include(stb)
if (BUILD_VIS)
    include(circe)
endif (BUILD_VIS)
if (BUILD_TESTS)
    include(catch2)
endif (BUILD_TESTS)
##########################################
##              source                  ##
##########################################
set(HEADERS
        helios/accelerators/list.h
        helios/common/bitmask_operators.h
        helios/common/io.h
        helios/common/globals.h
        helios/common/result.h
        helios/cameras/perspective_camera.h
        helios/core/aggregate.h
        helios/core/camera.h
        helios/core/film.h
        helios/core/filter.h
        helios/core/interaction.h
        helios/core/light.h
        helios/core/mem.h
        helios/core/primitive.h
        helios/core/renderer.cuh
        #        helios/core/sampler.h
        helios/core/sampling.h
        helios/core/scene.h
        helios/core/shape.h
        helios/core/spectrum.h
        helios/geometry/animated_transform.h
        helios/geometry/bounds.h
        helios/geometry/ray.h
        helios/geometry/transform.h
        helios/geometry/utils.h
        helios/integrators/whitted_integrator.h
        helios/lights/point.h
        helios/samplers/pixel_sampler.h
        helios/samplers/sample_pool.h
        helios/samplers/stratified_sampler.h
        helios/shapes/intersection.h
        helios/shapes/shapes.h
        helios/shapes/sphere.h
        )

set(SOURCES
        helios/accelerators/list.cpp
        helios/common/globals.cpp
        helios/common/io.cpp
        helios/cameras/perspective_camera.cpp
        helios/core/camera.cpp
        helios/core/film.cu
        helios/core/filter.cpp
        helios/core/interaction.cpp
        helios/core/light.cpp
        helios/core/mem.cpp
        helios/core/primitive.cpp
        #        helios/core/sampler.cpp
        helios/core/sampling.cpp
        helios/core/scene.cpp
        helios/core/shape.cpp
        helios/core/spectrum.cpp
        helios/geometry/animated_transform.cpp
        helios/geometry/ray.cu
        helios/geometry/transform.cpp
        helios/geometry/io.cpp
        helios/lights/point.cpp
        helios/samplers/pixel_sampler.cpp
        helios/samplers/sample_pool.cpp
        helios/samplers/stratified_sampler.cu
        helios/shapes/intersection.cpp
        helios/shapes/sphere.cu
        )

##########################################
##              helios                  ##
##########################################
add_library(helios STATIC ${SOURCES} ${HEADERS})
set_target_properties(helios PROPERTIES
        OUTPUT_NAME "helios"
        FOLDER "HELIOS")

set_source_files_properties(
        helios/integrators/sampler_integrator.cuh
        ${SOURCES}
        PROPERTIES LANGUAGE CUDA)

add_dependencies(helios hermes)
target_link_libraries(helios PUBLIC ${HERMES_LIBRARIES})
target_compile_definitions(helios PUBLIC
#        -DHELIOS_DEBUG_DATA=1
        -DENABLE_CUDA=1
        -D__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__=1)
target_include_directories(helios PUBLIC ${HERMES_INCLUDES} ${HELIOS_SOURCE_DIR} ${STB_INCLUDES})
target_compile_options(helios PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
        --generate-line-info
        --use_fast_math
        #-arch=sm_50
        -Xcompiler -pg
        --relocatable-device-code=true
        #    -–extended-lambda
        >)
set_target_properties(helios PROPERTIES
        LINKER_LANGUAGE CUDA
        CMAKE_CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS OFF
        POSITION_INDEPENDENT_CODE ON
        CUDA_STANDARD 17
        CUDA_STANDARD_REQUIRED ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        )
##########################################
##              tests                   ##
##########################################
if (BUILD_TESTS)
    add_subdirectory(tests)
endif (BUILD_TESTS)
##########################################
##               vis                    ##
##########################################
if (BUILD_VIS)
    add_subdirectory(vis)
endif (BUILD_VIS)
##########################################
##           documentation              ##
##########################################
if (BUILD_DOCS)
    # TODO
endif (BUILD_DOCS)
