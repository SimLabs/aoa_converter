add_compile_options("/FIstdafx.h")

INCLUDE_DIRECTORIES(./headers)

FILE(GLOB SRC_FILES sources/*.cpp)

SET(TARGET_SRC
    ${SRC_FILES}
)

FILE(GLOB_RECURSE  HEADER_FILES
    headers/*.h

    ${ACTUAL_3RDPARTY_DIR}/include/alloc/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/binary/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/common/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/cpp_utils/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/geometry/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/logger/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/reflection/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/serialization/*.h
)

MESSAGE("${HEADER_FILES}")

SET(TARGET_H
	${HEADER_FILES}
)

#### end var setup  ###

set(Boost_NO_BOOST_CMAKE ON)
set(BOOST_ROOT ${ACTUAL_3RDPARTY_DIR})
set(Boost_USE_MULTITHREADED      ON)
set(BOOST_INCLUDEDIR ${ACTUAL_3RDPARTY_DIR}/include/boost)
set(BOOST_LIBRARYDIR ${ACTUAL_3RDPARTY_DIR}/lib/boost)
find_package(Boost COMPONENTS system filesystem REQUIRED)

INCLUDE_DIRECTORIES (
#     ${WOMBAT_INCLUDE_DIR}
#     ${LOGGER_INCLUDE_DIR}
#     ${CONFIG_INCLUDE_DIR}
#     ${MODEL_STRUCTURE_INCLUDE_DIR}
     ${GEOMETRY_INCLUDE_DIR}
     ${COMMON_INCLUDE_DIR}
     ${RAPIDJSON_INCLUDE_DIR}
#     ${ALLOC_INCLUDE_DIR}
#     ${CPP_UTILS_INCLUDE_DIR}
#     ${BINARY_INCLUDE_DIR}
#     ${SERIALIZATION_INCLUDE_DIR}
#     ${REFLECTION_INCLUDE_DIR}
#     ${INFO_IO_INCLUDE_DIR}
#     ${BOOST_INCLUDEDIR}
)
#
LINK_DIRECTORIES (
#     ${WOMBAT_LIBRARY_DIR}
#     ${LOGGER_LIBRARY_DIR}
#     ${CONFIG_LIBRARY_DIR}
#     ${ALLOC_LIBRARY_DIR}
#     ${INFO_IO_LIBRARY_DIR}
${BOOST_LIBRARYDIR})
#
# SET(TARGET_LIBRARIES_VARS
#     WOMBAT_LIBRARY
#     LOGGER_LIBRARY
#     CONFIG_LIBRARY
#     ALLOC_LIBRARY
#     INFO_IO_LIBRARY
#     )

SET(TARGET_EXTERNAL_LIBRARIES osgSim)

## define macros
add_definitions(-DCG_PRIMITIVES)
add_definitions(-DDISABLE_CUSTOM_ALLOCATOR)
# SET (CG_PRIMITIVES true)
# SET (WIN32_LEAN_AND_MEAN true)
# SET (NOMINMAX true)

## add precompiled header
## add totum libraries & includes

SETUP_PLUGIN(aoa)
