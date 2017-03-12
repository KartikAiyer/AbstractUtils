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


/* The main thread set its priority to PRI_MIN and creates 7 threads
   (thread 1..7) with priorities PRI_MIN + 3, 6, 9, 12, ...
   The main thread initializes 8 locks: lock 0..7 and acquires lock 0.

   When thread[i] starts, it first acquires lock[i] (unless i == 7.)
   Subsequently, thread[i] attempts to acquire lock[i-1], which is held by
   thread[i-1], except for lock[0], which is held by the main thread.
   Because the lock is held, thread[i] donates its priority to thread[i-1],
   which donates to thread[i-2], and so on until the main thread
   receives the donation.

   After threads[1..7] have been created and are blocked on locks[0..7],
   the main thread releases lock[0], unblocking thread[1], and being
   preempted by it.
   Thread[1] then completes acquiring lock[0], then releases lock[0],
   then releases lock[1], unblocking thread[2], etc.
   Thread[7] finally acquires & releases lock[7] and exits, allowing
   thread[6], then thread[5] etc. to run and exit until finally the
   main thread exits.

   In addition, interloper threads are created at priority levels
   p = PRI_MIN + 2, 5, 8, 11, ... which should not be run until the
   corresponding thread with priority p + 1 has finished.

   Written by Godmar Back <gback@cs.vt.edu> */

#include <embUnit/embUnit.h>
#include <ThreadInterface.h>
#include <MutexInterface.h>
#include <Logable.h>
#include <stdio.h>

#define NESTING_DEPTH               (8)
#define THREAD_STACK_SIZE           ( 1 << 14 )
#define INTERLOPER_STACK_SIZE       ( 1 << 14 )
#define PRI_MIN       0

typedef struct _LockPair
{
  KMutex *second;
  KMutex *first;
}LockPair;

typedef struct _StaticTestData
{
  uint8_t stacks[NESTING_DEPTH][ THREAD_STACK_SIZE ];
  uint8_t interloperStacks[NESTING_DEPTH][INTERLOPER_STACK_SIZE];
  KThread threads[ NESTING_DEPTH ];
  KThread interloperThreads[ NESTING_DEPTH ];
  uint32_t currentThreadIndexToRelease;
}TestData;

typedef struct _DonorThreadParams
{
  KThread* pThread;
  uint32_t i;
  LockPair lockPair;
  TestData* pTestData;
}DonorThreadParams;

typedef struct _InterloperParams
{
  KThread* pThread;
  uint32_t i;
  TestData* pTestData;
}InterloperParams;

static void DonorThreadFunction( void *args );
static void InterloperThreadFunction( void *args );
static TestData s_data = { 0 };

static void Setup( void )
{
}

static void TearDown( void )
{
}

static void StartThread( void* arg )
{
  KMutex locks[NESTING_DEPTH - 1];
  DonorThreadParams donorParams[ NESTING_DEPTH ];
  InterloperParams interloperParams[ NESTING_DEPTH ];
  TestData* pTestData = ( TestData* ) arg;

  for (uint32_t i = 0; i < NESTING_DEPTH - 1; i++) {
    char tmp[30];
    bool workerMutexCreateResult = false;
    sprintf( tmp , "Mutex %d", i );
    workerMutexCreateResult = KMutexCreate( &locks[ i ], tmp );
  }

  KMutexLock( &locks[0], WAIT_FOREVER );
  LOG( "%s got lock, My Priority: %d", s_data.threads[0].threadName,
       KThreadGetPriority( &pTestData->threads[ 0 ] ) );

  for ( uint32_t i = 1; i < NESTING_DEPTH; i++)
  {
    char name[16];
    int thread_priority;
    snprintf (name, sizeof name, "thread %d", i);
    thread_priority = PRI_MIN + i * 3;

    donorParams[ i ].lockPair.first = i < NESTING_DEPTH - 1 ? locks + i : NULL;
    donorParams[ i ].lockPair.second = locks + i - 1;
    donorParams[ i ].i = i;
    donorParams[ i ].pThread = &pTestData->threads[ i ];
    donorParams[ i ].pTestData = pTestData;
    KThreadCreateParams params = {
        .fn = DonorThreadFunction,
        .pStack = pTestData->stacks[ i ],
        .pThreadName = name,
        .stackSizeInBytes = sizeof( pTestData->stacks[ i ] ),
        .threadArg = &donorParams[i],
        .threadPriority = thread_priority
    };
    LOG( "Starting %s with priority: %d", name, thread_priority );
    TEST_ASSERT( KThreadCreate( &pTestData->threads[ i ], &params ) );
    /*
    LOG ("%s should have priority %d.  Actual priority: %d.",
         name, thread_priority, KThreadGetPriority( &pTestData->threads[ i ] ) );
    snprintf (name, sizeof name, "interloper %d", i);
    interloperParams[ i ].i = i;
    interloperParams[ i ].pThread = &pTestData->interloperThreads[ i ];
    interloperParams[ i ].pTestData = pTestData;
    KThreadCreateParams paramsInterloper = {
        .fn = InterloperThreadFunction,
        .pStack = pTestData->interloperStacks[ i ],
        .pThreadName = name,
        .stackSizeInBytes = sizeof( pTestData->interloperStacks[ i ] ),
        .threadArg = &interloperParams[ i ],
        .threadPriority = thread_priority - 1,
    };
    TEST_ASSERT( KThreadCreate( &pTestData->interloperThreads[ i ], &paramsInterloper ) );
     */
  }
  TEST_ASSERT_EQUAL_INT( NESTING_DEPTH - 1, pTestData->currentThreadIndexToRelease );
  KMutexUnlock( &locks[ 0 ] );
  /*
  LOG("%s finishing with priority %d.\n", KThreadGetName( &pTestData->threads[ 0 ] ),
       KThreadGetPriority( &pTestData->threads[ 0 ] ) );
       */
  TEST_ASSERT( KThreadJoin( &pTestData->threads[ 1 ] ) && KThreadJoin( &pTestData->interloperThreads[ 1 ] ) );
}

static void TestPriorityDonateChain( void )
{
  TestData* pTestData = &s_data;
  pTestData->currentThreadIndexToRelease = 0;
  KThreadCreateParams startThread = {
      .fn = StartThread,
      .pStack = pTestData->stacks[ 0 ],
      .pThreadName = "StartThread",
      .stackSizeInBytes = THREAD_STACK_SIZE,
      .threadArg = pTestData,
      .threadPriority = PRI_MIN,
  };
  pTestData->currentThreadIndexToRelease = 0;
  bool startThreadCreateResult = KThreadCreate( &pTestData->threads[ 0 ], &startThread );
  TEST_ASSERT( startThreadCreateResult );
  //Test ends when StartThread Ends
  bool waitForStartThreadEnd = KThreadJoin( &pTestData->threads[ 0 ] );
  TEST_ASSERT( waitForStartThreadEnd );
}

static void
DonorThreadFunction( void *args )
{
  DonorThreadParams* pParams = ( DonorThreadParams* )args;

  if (pParams->lockPair.first)
    KMutexLock(pParams->lockPair.first, WAIT_FOREVER );
  LOG( "%s priority %d, Actual priority: %d\n",
       KThreadGetName( pParams->pThread ), KThreadGetPriority( pParams->pThread ) );
  TEST_ASSERT_EQUAL_INT( pParams->i - 1, pParams->pTestData->currentThreadIndexToRelease );
  pParams->pTestData->currentThreadIndexToRelease = pParams->i;
  KMutexLock(pParams->lockPair.second, WAIT_FOREVER );
  TEST_ASSERT_EQUAL_INT( NESTING_DEPTH - pParams->i, pParams->pTestData->currentThreadIndexToRelease );
  pParams->pTestData->currentThreadIndexToRelease = NESTING_DEPTH - ( pParams->i + 1 );
  KMutexUnlock(pParams->lockPair.second);
  /*
  LOG ("%s should have priority %d. Actual priority: %d\n",
       KThreadGetName( pParams->pThread ), (NESTING_DEPTH - 1) * 3,
       KThreadGetPriority( pParams->pThread ) );
       */

  if (pParams->lockPair.first)
    KMutexUnlock(pParams->lockPair.first);

/*
  LOG("%s finishing with priority %d.\n", KThreadGetName( pParams->pThread ),
       KThreadGetPriority( pParams->pThread ) );
       */
}

static void
InterloperThreadFunction( void *args )
{
  InterloperParams* pParams = ( InterloperParams* )args;
  LOG("%s finished.", KThreadGetName(pParams->pThread) );
}

TestRef PriorityDonateChainTest()
{
  EMB_UNIT_TESTFIXTURES( fixture ) {
      new_TestFixture( "PriorityDonateChain", TestPriorityDonateChain )
  };
  EMB_UNIT_TESTCALLER( PriorityDonateChainCaller,"ThreadPriorityBasedTests", Setup, TearDown, fixture );
  return (TestRef)&PriorityDonateChainCaller;
}

