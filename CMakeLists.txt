cmake_minimum_required( VERSION 2.6.0 )
project( AbstractUtils C )

set( INTERFACE ${PROJECT_SOURCE_DIR}/Interface )
set( DEFAULT_IMPL ${PROJECT_SOURCE_DIR}/DefaultImplementations )
set( POSIX_INTERFACE ${PROJECT_SOURCE_DIR}/PosixInterface )
set( UTILS ${PROJECT_SOURCE_DIR}/Utils )

include_directories( ${INTERFACE} )
include_directories( ${DEFAULT_IMPL} )
include_directories( ${POSIX_INTERFACE} )
include_directories( ${UTILS} )

file( GLOB DEF_SOURCES ${DEFAULT_IMPL}/*.c ${DEFAULT_IMPL}/*.h )
file( GLOB INTERFACE_HEADERS ${INTERFACE}/*.h )
file( GLOB POSIX_SOURCES ${POSIX_INTERFACE}/*.c ${POSIX_INTERFACE}/*.h )
file( GLOB UTIL_SOURCES ${UTILS}/*.c ${UTILS}/*.h )

add_library( AbstractUtils STATIC ${INTERFACE} ${DEF_SOURCES} ${POSIX_SOURCES} ${UTIL_SOURCES} )

add_subdirectory(${PROJECT_SOURCE_DIR}/Tests)
