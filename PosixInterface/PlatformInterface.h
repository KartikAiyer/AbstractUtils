/**
 * The MIT License (MIT)
 *
 * Copyright (c) <2014> <Kartik Aiyer>
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
#ifndef __PLATFORM_INTERFACE_H__
#define __PLATFORM_INTERFACE_H__

#include <pthread.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void ( *KThreadCallback )( void* arg );

#define THREAD_NAME_MAX_SIZE          ( 20 )
#define THREAD_SANITY_CHECK           ( 0xDEADBEEF )
typedef struct _KThread
{
  pthread_t pthread;
  KThreadCallback fn;
  void* arg;
  bool isComplete;
  char threadName[ THREAD_NAME_MAX_SIZE ];
  uint32_t sanity;
}KThread;

typedef struct _KSema {
  sem_t *pNamedSema;
  char* pSemaphoreName;
}KSema;

typedef pthread_mutex_t KMutex;

#ifdef __cplusplus
}
#endif
#endif // __PLATFORM_INTERFACE_H__
