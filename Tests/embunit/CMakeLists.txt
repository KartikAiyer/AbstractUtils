cmake_minimum_required( VERSION 2.6.0 )
project( embUnit C )

set( EMBUNIT ${PROJECT_SOURCE_DIR}/embUnit )
set( SAMPLES ${PROJECT_SOURCE_DIR}/samples )
set( TESTS ${PROJECT_SOURCE_DIR}/tests )
set( TEXTUI ${PROJECT_SOURCE_DIR}/textui )
set( TOOLS ${PROJECT_SOURCE_DIR}/tools )

include_directories( ${PROJECT_SOURCE_DIR} )

file( GLOB EMB_SOURCE ${EMBUNIT}/*.c ${EMBUNIT}/*.h )
file( GLOB SAMPLE_SOURCE ${SAMPLES}/*.c ${SAMPLES}/*.h )
file( GLOB TESTS_SOURCE ${TESTS}/*.c ${TESTS}/*.h )
file( GLOB TEXTUI_SOURCE ${TEXTUI}/*.c ${TEXTUI}/*.h )
file( GLOB TOOLS_SOURCE ${TOOLS}/*.c ${TOOLS}/*.h )

set( CMAKE_C_FLAGS "-m32" )
set( CMAKE_EXE_LINKER_FLAGS "-m32" )

add_library( embUnit STATIC ${EMB_SOURCE} )
add_executable( embUnitSamples ${SAMPLE_SOURCE} )
target_link_libraries( embUnitSamples embUnit )
