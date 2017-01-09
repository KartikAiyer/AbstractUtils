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
#include <string.h>
#include <unistd.h>

typedef struct _ThreadTest1Data
{
  uint8_t value;
  KMutex mtx;
} Test1Data;

static Test1Data s_tst1 = { 0 };

static void TestThreadFunction( void *arg )
{
  usleep( 1000000 );
  s_tst1.value = 1;
}

static void SetUp()
{

}
static void TearDown()
{

}

static void ThreadApiTest()
{
  KThread thread;
  KTHREAD_CREATE_PARAMS( threadParams, "Test Thread", TestThreadFunction, NULL, NULL, 1024 * 16, 100 );
  bool retval = KThreadCreate( &thread, KTHREAD_PARAMS( threadParams ) );
  TEST_ASSERT( retval );
  retval = KThreadJoin( &thread );
  TEST_ASSERT( retval );
  TEST_ASSERT( s_tst1.value == 1 );
  retval = KThreadDelete( &thread );
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

static void TestBasicPremption()
{
  memset( &s_tst1, 0, sizeof( s_tst1 ));
  if( KMutexCreate( &s_tst1.mtx, "ThreadPremptTestMtx" )) {
    s_tst1.value = 0;
    KThread threadMid;
    KThread threadHi;
    KTHREAD_CREATE_PARAMS( threadParamsMid,
                           "ThreadPremptTestMid",
                           PremptTestThreadMid,
                           NULL,
                           NULL,
                           1024 * 16,
                           100 );
    KTHREAD_CREATE_PARAMS( threadParamsHi,
                           "ThreadPremptTestHi",
                           PremptTestThreadHi,
                           NULL,
                           NULL,
                           1024 * 16,
                           120 );
    if( KThreadCreate( &threadMid, KTHREAD_PARAMS( threadParamsMid ) )) {
      if( KThreadCreate( &threadHi, KTHREAD_PARAMS( threadParamsHi ) )) {
        TEST_ASSERT( KThreadJoin( &threadMid ) );
        TEST_ASSERT( KThreadJoin( &threadHi ) );
        TEST_ASSERT( KThreadDelete( &threadMid ) );
        TEST_ASSERT( KThreadDelete( &threadHi ) );
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

