cmake_minimum_required( VERSION 2.6.0 )
project( AbstractUtilTest CXX C )

set( GTEST_DIR ${PROJECT_SOURCE_DIR}/gtest )
set( CMAKE_C_FLAGS "-m32" )
set( CMAKE_CXX_FLAGS "-m32" )
set( CMAKE_EXE_LINKER_FLAGS "-m32" )

add_subdirectory( ${GTEST_DIR} )
add_subdirectory( ${PROJECT_SOURCE_DIR}/embUnit )
include_directories( ${GTEST_DIR}/include )

file( GLOB TEST_SRC ${PROJECT_SOURCE_DIR}/*.cc ${PROJECT_SOURCE_DIR}/*.c )

add_executable( tests ${TEST_SRC} )
target_link_libraries( tests gtest AbstractUtils )
