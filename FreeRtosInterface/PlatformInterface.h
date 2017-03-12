/**
 * The MIT License (MIT)
 *
 * Copyright (c) <2017> <Kartik Aiyer>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ABSTRACTUTILS_PLATFORMINTERFACE_H_H
#define ABSTRACTUTILS_PLATFORMINTERFACE_H_H

#ifdef __cplusplus
extern "C" {
#endif

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <portmacro.h>
#include <task.h>
#include <stdbool.h>
#include <stdint.h>
#include <semphr.h>
#include <InterfacePrivateCommon.h>

typedef void (*KThreadCallback)(void* arg);

#define THREAD_NAME_MAX_SIZE          ( configMAX_TASK_NAME_LEN )
#define THREAD_SANITY_CHECK           ( 0xDEADBEEF )

typedef struct {
  StaticSemaphore_t sem;
  SemaphoreHandle_t hSem;
  char semaphoreName[ THREAD_NAME_MAX_SIZE ];
}KSema;

typedef struct {
  StaticSemaphore_t sem;
  SemaphoreHandle_t hSem;
  char mutexName[ THREAD_NAME_MAX_SIZE ];
}KMutex;

typedef struct {
  StaticTask_t task;
  TaskHandle_t hTask;
  KThreadCallback fn;
  void* arg;
  bool isComplete;
  char threadName[ THREAD_NAME_MAX_SIZE ];
  uint32_t sanity;
  KMutex joinMutex;
  TaskHandle_t hJoinRequestTask;
}KThread;

inline uint32_t TimeoutInMsToTickConverter( uint32_t timeoutInMs )
{
  uint32_t retval = 0;
  if( timeoutInMs == NO_SLEEP ){
    retval = 0;
  }  else if( timeoutInMs == WAIT_FOREVER) {
    retval = portMAX_DELAY;
  } else {
    retval = timeoutInMs / portTICK_PERIOD_MS;
    retval = ( !retval ) ? 1 : retval;
  }
  return retval;
}

#define SEMANTIC_THREAD_PRIORITY_LOWEST    ( 0 )
#define SEMANTIC_THREAD_PRIORITY_HIGHEST   ( configMAX_PRIORITIES	- 1)
#define SEMANTIC_THREAD_PRIORITY_MID       ( ( SEMANTIC_THREAD_PRIORITY_HIGHEST + SEMANTIC_THREAD_PRIORITY_LOWEST ) / 2 )
#define SEMANTIC_THREAD_PRIORITY_LOW       ( ( SEMANTIC_THREAD_PRIORITY_MID + SEMANTIC_THREAD_PRIORITY_LOWEST ) / 2 )
#define SEMANTIC_THREAD_PRIORITY_HIGH      ( ( SEMANTIC_THREAD_PRIORITY_HIGHEST + SEMANTIC_THREAD_PRIORITY_MID ) / 2 ) 

#ifdef __cplusplus
}
#endif


#endif //ABSTRACTUTILS_PLATFORMINTERFACE_H_H
