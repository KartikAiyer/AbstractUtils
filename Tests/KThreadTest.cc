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

#include <gtest/gtest.h>
#include <ThreadInterface.h>
#include <MutexInterface.h>
#include <Logable.h>

typedef struct _ThreadTest1Data
{
  uint8_t value;
  KMutex mtx;
}Test1Data;

static Test1Data s_tst1 = { 0 };
static void TestThreadFunction( void* arg )
{
  usleep( 1000000 );
  s_tst1.value = 1;
}

TEST( KThreadTest, ThreadApiTest )
{
  KThread thread;
  KTHREAD_CREATE_PARAMS( threadParams, "Test Thread", TestThreadFunction, NULL, 1024 * 8, 100 );
  bool retval = KThreadCreate( &thread, KTHREAD_PARAMS( threadParams ) );
  ASSERT_TRUE( retval );
  retval = KThreadJoin( &thread );
  ASSERT_TRUE( retval );
  ASSERT_TRUE( s_tst1.value == 1 );
  retval = KThreadDelete( &thread );
  ASSERT_TRUE( retval );
}

static void PremptTestThreadHi( void* arg )
{
  KMutexLock( &s_tst1.mtx, WAIT_FOREVER );
  s_tst1.value = 20;
  KMutexUnlock( &s_tst1.mtx );
}

static void PremptTestThreadMid( void* arg )
{
  uint32_t val = 0;
  bool keepRunning = true;
  while( keepRunning ) {
    LOG( "%s(): Running... ", __FUNCTION__ );
    KMutexLock(&s_tst1.mtx, WAIT_FOREVER);
    val = s_tst1.value;
    KMutexUnlock(&s_tst1.mtx);
    keepRunning = ( val == 0 ) ? true : false;
  }
}

TEST( KThreadTest, TestBasicPremption )
{
  memset( &s_tst1, 0, sizeof( s_tst1 ) );
  if( KMutexCreate( &s_tst1.mtx, "ThreadPremptTestMtx" ) ) {
    s_tst1.value = 0;
    KThread threadMid;
    KThread threadHi;
    KTHREAD_CREATE_PARAMS( threadParamsMid,
                           "ThreadPremptTestMid",
                           PremptTestThreadMid,
                           NULL,
                           1024 * 8,
                           100 );
    KTHREAD_CREATE_PARAMS( threadParamsHi,
                           "ThreadPremptTestHi",
                           PremptTestThreadHi,
                           NULL,
                           1024 * 8,
                           120 );
    if( KThreadCreate( &threadMid, KTHREAD_PARAMS( threadParamsMid ) ) ){
        sleep( 1 );
        if( KThreadCreate( &threadHi, KTHREAD_PARAMS( threadParamsHi ) ) ) {
            ASSERT_TRUE( KThreadJoin( &threadMid ) );
            ASSERT_TRUE( KThreadJoin( &threadHi ) );
            ASSERT_TRUE( KThreadDelete( &threadMid ) );
            ASSERT_TRUE( KThreadDelete( &threadHi ) );
        }
    }
    KMutexDelete( &s_tst1.mtx );
  }
}



