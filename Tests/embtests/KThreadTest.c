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
#include <MutexInterface.h>

typedef struct _ThreadTest1Data
{
  uint8_t value;
  KMutex mtx;
} Test1Data;

static Test1Data s_tst1 = { 0 };

static void TestThreadFunction( void *arg )
{
 // usleep( 1000000 );
  s_tst1.value = 1;
}

static void SetUp(void)
{

}
static void TearDown(void)
{

}

typedef struct _ThreadData
{
  KThread thread;
  uint8_t stack[ 1024 ];
}ThreadData;


static ThreadData s_testThreadA;
static void ThreadApiTest(void)
{
  KTHREAD_CREATE_PARAMS( s_testThreadAParams, 
                         "Test Thread", 
                         TestThreadFunction, 
                         NULL, 
                         s_testThreadA.stack, 
                         sizeof( s_testThreadA.stack ), 
                         SEMANTIC_THREAD_PRIORITY_MID );
  bool retval = KThreadCreate( &s_testThreadA.thread, KTHREAD_PARAMS( s_testThreadAParams ) );
  TEST_ASSERT( retval );
  retval = KThreadJoin( &s_testThreadA.thread );
  TEST_ASSERT( retval );
  TEST_ASSERT( s_tst1.value == 1 );
  retval = KThreadDelete( &s_testThreadA.thread );
  TEST_ASSERT( retval );
}

static void PremptTestThreadHi( void *arg )
{
  KMutexLock( &s_tst1.mtx, WAIT_FOREVER);
  s_tst1.value = 20;
  KMutexUnlock( &s_tst1.mtx );
}

static void PremptTestThreadMid( void *arg )
{
  uint32_t val = 0;
  bool keepRunning = true;
  while( keepRunning ) {
    KMutexLock( &s_tst1.mtx, WAIT_FOREVER);
    val = s_tst1.value;
    KMutexUnlock( &s_tst1.mtx );
    keepRunning = ( val == 0 ) ? true : false;
  }
}

static ThreadData s_testThreadMid;
static ThreadData s_testThreadHi;
static void TestBasicPremption( void )
{
  memset( &s_tst1, 0, sizeof( s_tst1 ));
  if( KMutexCreate( &s_tst1.mtx, "ThreadPremptTestMtx" )) {
    s_tst1.value = 0;
    KTHREAD_CREATE_PARAMS( threadParamsMid,
                           "ThreadPremptTestMid",
                           PremptTestThreadMid,
                           NULL,
                           s_testThreadMid.stack,
                           sizeof( s_testThreadMid.stack ),
                           SEMANTIC_THREAD_PRIORITY_LOWEST);
    KTHREAD_CREATE_PARAMS( threadParamsHi,
                           "ThreadPremptTestHi",
                           PremptTestThreadHi,
                           NULL,
                           s_testThreadMid.stack,
                           sizeof( s_testThreadMid.stack ),
                           SEMANTIC_THREAD_PRIORITY_HIGH );
    if( KThreadCreate( &s_testThreadMid.thread, KTHREAD_PARAMS( threadParamsMid ) )) {
      if( KThreadCreate( &s_testThreadHi.thread, KTHREAD_PARAMS( threadParamsHi ) )) {
        TEST_ASSERT( KThreadJoin( &s_testThreadMid.thread ) );
        TEST_ASSERT( KThreadJoin( &s_testThreadHi.thread ) );
        TEST_ASSERT( KThreadDelete( &s_testThreadMid.thread ) );
        TEST_ASSERT( KThreadDelete( &s_testThreadHi.thread ) );
      }
    }
    KMutexDelete( &s_tst1.mtx );
  }
}

TestRef KThreadTest_ApiTests()
{
  EMB_UNIT_TESTFIXTURES(fixtures) {
    new_TestFixture( "TreadApiTest", ThreadApiTest ),
    new_TestFixture( "BasicPremption", TestBasicPremption )

  };
  EMB_UNIT_TESTCALLER( KThreadBasic, "KThreadBasic", SetUp, TearDown, fixtures );
  return (TestRef)&KThreadBasic;
}

