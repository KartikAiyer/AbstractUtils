cmake_minimum_required( VERSION 3.0.0 )
project( FreeRTOS )

macro( FreeRtosCore )
  set( FRTOS_CORE_SOURCE
       ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/list.c
       ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/tasks.c
       ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/queue.c
       ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/timers.c
       ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/event_groups.c )
  file( GLOB FRTOS_CORE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/include/*.h)
endmacro( FreeRtosCore )

if( WIN32 AND HOSTOS_PORT )
  MESSAGE( "FreeRTOS Windows port" )
  FreeRtosCore()
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/include)
  set( FRTOS_PLATFORM_SOURCE
       ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/MSVC-MingW/port.c )
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/MSVC-MingW )
  set( FRTOS_INCLUDE_PATH ${FRTOS_INCLUDE_PATH}
                          ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/MSVC-MingW )
else( WIN32 AND HOSTOS_PORT )
  MESSAGE( FATAL_ERROR "FreeRTOS build is not supported in other flavors" )
endif( WIN32 AND HOSTOS_PORT )

configure_file( ${PROJECT_SOURCE_DIR}/FreeRTOSConfig.h.in
                ${PROJECT_BINARY_DIR}/include/FreeRTOSConfig.h )
list( APPEND FRTOS_CORE_HEADERS ${PROJECT_BINARY_DIR}/include/FreeRTOSConfig.h 
								${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/MSVC-MingW/portmacro.h )
add_library( FreeRTOS STATIC ${FRTOS_CORE_SOURCE} ${FRTOS_CORE_HEADERS} ${FRTOS_PLATFORM_SOURCE} )
target_include_directories( FreeRTOS PUBLIC ${PROJECT_BINARY_DIR}/include )
set( FRTOS_INCLUDE_PATH ${FRTOS_INCLUDE_PATH}
                        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/include
                        ${PROJECT_BINARY_DIR}/include
                        PARENT_SCOPE )
