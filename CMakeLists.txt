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

if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    find_package( Threads )
    add_definitions( -D_GNU_SOURCE -DLINUX_PTHREAD )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

file( GLOB DEF_SOURCES ${DEFAULT_IMPL}/*.c ${DEFAULT_IMPL}/*.h )
file( GLOB INTERFACE_SOURCES ${INTERFACE}/*.h ${INTERFACE}/*.c )
file( GLOB POSIX_SOURCES ${POSIX_INTERFACE}/*.c ${POSIX_INTERFACE}/*.h )
file( GLOB UTIL_SOURCES ${UTILS}/*.c ${UTILS}/*.h )

set( SOURCES ${INTERFACE_SOURCES} ${POSIX_SOURCES} ${DEF_SOURCES} ${UTIL_SOURCES} )
set( CMAKE_C_FLAGS -m32)
set( CMAKE_EXE_LINKER_FLAGS -m32 )
add_library( AbstractUtils STATIC ${SOURCES} )
if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    target_link_libraries( AbstractUtils ${CMAKE_THREAD_LIBS_INIT} )
endif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

add_subdirectory(${PROJECT_SOURCE_DIR}/Tests)
