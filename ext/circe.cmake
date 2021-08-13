if (CIRCE_INCLUDE_PATH AND CIRCE_LIB_PATH)
    MESSAGE("using circe from " ${CIRCE_INCLUDE_PATH} " and " ${CIRCE_LIB_PATH})
    set(CIRCE_INCLUDES ${CIRCE_INCLUDE_PATH})

    set(CIRCE_LIBRARIES
            ${CIRCE_LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}circe${CMAKE_STATIC_LIBRARY_SUFFIX}
            ${CIRCE_LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}glad${CMAKE_STATIC_LIBRARY_SUFFIX}
            ${CIRCE_LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX}
            ${CIRCE_LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}tinyobjloader${CMAKE_STATIC_LIBRARY_SUFFIX}
            dl
            pthread
            X11
            )

    add_library(circe STATIC
            ${CIRCE_LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}circe${CMAKE_STATIC_LIBRARY_SUFFIX}
            )
    set_target_properties(circe PROPERTIES
            LINKER_LANGUAGE CXX)
else ()
    include(ExternalProject)
    ExternalProject_Add(
            circe PREFIX circe
            URL "https://github.com/filipecn/circe/archive/main.zip"
            # URL_HASH SHA1=fe17a0610a239311a726ecabcd2dbd669fb24ca8
            CMAKE_ARGS
            "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DINSTALL_PATH=install"
            "-DHERMES_LIB_PATH=${HERMES_LIBRARIES}"
            "-DHERMES_INCLUDE_PATH=${HERMES_INCLUDES}"
            "-DBUILD_WITH_CUDA=OFF"
            CMAKE_CACHE_ARGS
            "-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}"
            "-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}"
    )

    ExternalProject_Get_Property(circe INSTALL_DIR)
    set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/circe)
    set(CIRCE_INCLUDES
            ${INSTALL_DIR}/install/include
            )


    set(CIRCE_LIBRARIES
            ${INSTALL_DIR}/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}circe${CMAKE_STATIC_LIBRARY_SUFFIX}
            ${INSTALL_DIR}/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glad${CMAKE_STATIC_LIBRARY_SUFFIX}
            ${INSTALL_DIR}/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX}
            ${INSTALL_DIR}/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tinyobjloader${CMAKE_STATIC_LIBRARY_SUFFIX}
            dl
            pthread
            X11
            )

    # append hermes to circe
    set(HERMES_INSTALL_DIR ${INSTALL_DIR}/src/circe-build/hermes/install)
    set(CIRCE_INCLUDES
            ${CIRCE_INCLUDES})
    #        ${HERMES_INSTALL_DIR}/include)
    set(CIRCE_LIBRARIES
            ${CIRCE_LIBRARIES})
    #        ${HERMES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}hermes${CMAKE_STATIC_LIBRARY_SUFFIX})
endif ()

set(CIRCE_INCLUDES ${CIRCE_INCLUDES} CACHE STRING "")
set(CIRCE_LIBRARIES ${CIRCE_LIBRARIES} CACHE STRING "")
