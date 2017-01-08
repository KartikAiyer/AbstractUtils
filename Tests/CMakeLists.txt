cmake_minimum_required( VERSION 2.6.0 )
project( AbstractUtilTest CXX C )

set( GTEST_DIR ${PROJECT_SOURCE_DIR}/gtest )
set( CMAKE_C_FLAGS "-m32" )
set( CMAKE_CXX_FLAGS "-m32" )
set( CMAKE_EXE_LINKER_FLAGS "-m32" )

#add_subdirectory( ${GTEST_DIR} )
#include_directories( ${GTEST_DIR}/include )

#file( GLOB GTEST_SRC ${PROJECT_SOURCE_DIR}/gtests/*.cc ${PROJECT_SOURCE_DIR}/gtests/*.c )

#add_executable( gtests ${GTEST_SRC} )
#target_link_libraries( gtests gtest AbstractUtils )

add_subdirectory( ${PROJECT_SOURCE_DIR}/embunit )
file( GLOB EMBTEST_SRC ${PROJECT_SOURCE_DIR}/embtests/*.c ${PROJECT_SOURCE_DIR}/embtests/*.h )
include_directories( ${PROJECT_SOURCE_DIR}/embunit/embUnit )
include_directories( ${PROJECT_SOURCE_DIR}/embunit )
add_executable( embtests ${EMBTEST_SRC} )
target_link_libraries( embtests embUnit AbstractUtils )
