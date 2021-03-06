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

#include <embUnit/embUnit.h>
#include <ThreadInterface.h>
#include <SemaphoreInterface.h>
#include <stdio.h>
#include <ConsoleLog.h>

typedef struct _SemPriWakeData
{
    KSema *pSema;
    uint32_t threadNum;
    uint32_t priority;
} SemPriWakeData;
static uint32_t s_currentThreadPriority = 0;

static void SetUp( void )
{}

static void TearDown( void )
{}

static void SemaphorePriorityWakeThread( void *arg )
{
  SemPriWakeData *pData = ( SemPriWakeData * ) arg;
  TEST_ASSERT( KSemaGet( pData->pSema, WAIT_FOREVER ));
  ConsoleLogLine( "Current: %u\n", pData->priority );
  TEST_ASSERT( s_currentThreadPriority == pData->priority );
  s_currentThreadPriority--;
  KSemaPut( pData->pSema );
}

static void SemaphorePrioritySemUpThread( void *arg )
{
  KSema *pSema = ( KSema * ) arg;
  KSemaPut( pSema );
}

typedef struct _ThreadData
{
  KThread thread;
  uint8_t threadStack[ 512 ];
}ThreadData;

static void SemaphorePriorityWake( void )
{
//Tests that the highest-priority thread waiting
// on a semaphore is the first to wake up.
#define NUM_OF_THREADS  ( SEMANTIC_THREAD_PRIORITY_HIGHEST + 1 )
  KSema sem;
  SemPriWakeData data[NUM_OF_THREADS];
  static ThreadData threads[NUM_OF_THREADS];
  static ThreadData lastThread;
  uint32_t priority = s_currentThreadPriority = SEMANTIC_THREAD_PRIORITY_HIGHEST;
  if( KSemaCreate( &sem, "SemPriWakeTest", 0 )) {
    for( uint32_t i = 0; i < NUM_OF_THREADS; i++ ) {
      char name[20] = {0};
      sprintf_s( name, sizeof( name ), "Thread %d", i );
      data[ i ].priority = priority;
      data[ i ].threadNum = i;
      data[ i ].pSema = &sem;
      KTHREAD_CREATE_PARAMS( threadParams,
                             name,
                             SemaphorePriorityWakeThread,
                             &data[ i ],
                             threads[ i ].threadStack,
                             sizeof( threads[ i ].threadStack ),
                             priority-- );
      if( !KThreadCreate( &threads[ i ].thread, KTHREAD_PARAMS( threadParams ) )) {
        TEST_FAIL( "Couldn't create test thread" );
        break;
      }
    }
    //Create the lowest priority thread that will up the semaphore
    KTHREAD_CREATE_PARAMS( threadLoPri,
                           "SemUpThread",
                           SemaphorePrioritySemUpThread,
                           &sem,
                           lastThread.threadStack,
                           sizeof( lastThread.threadStack ),
                           --priority );
    if( !KThreadCreate( &lastThread.thread, KTHREAD_PARAMS( threadLoPri ) )) {
      TEST_FAIL( "Couldn't create lowest priority thread" );
    }
    KThreadJoin( &lastThread.thread );
    for( uint32_t i = 0; i < NUM_OF_THREADS ; i++ ) {
      KThreadJoin( &threads[ i ].thread );
    }
    KThreadDelete( &lastThread.thread );
    for( uint32_t i = 0; i < NUM_OF_THREADS; i++ ) {
      KThreadDelete( &threads[ i ].thread );
    }
    KSemaDelete( &sem );
  }
#undef NUM_OF_THREADS
}

TestRef PriorityWakeTest()
{
  EMB_UNIT_TESTFIXTURES( fixtures ) {
    new_TestFixture( "SemaphoreWakesHighestPriorityThread", SemaphorePriorityWake )
  };
  EMB_UNIT_TESTCALLER( PriorityWakeCaller, "ThreadPriorityBasedTests", SetUp, TearDown, fixtures );
  return (TestRef)&PriorityWakeCaller;
}

