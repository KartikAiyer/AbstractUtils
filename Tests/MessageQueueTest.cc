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
#include <iostream>
#include <MessageQueue.h>
#include <ThreadInterface.h>

class MsgQTest : public ::testing::Test
{
public:
  static void TestFunction( void* arg )
  {
    MsgQTest *pTest = ( MsgQTest* )arg;
    if ( pTest->action ) {
      pTest->action( arg );
    }
  }
  void SetupQueue( void (*act)(void* arg), uint32_t queueSize )
  {
    //KThreadCreateParams params =  { .pThreadName = "TestThread", .fn = TestFunction, .stackSizeInBytes = 1024 * 6, .threadPriority = 0, .threadArg = this };
    KTHREAD_CREATE_PARAMS( thrPar, "TestThread", TestFunction, this, 1024*8, 0 );
    queueStore = ( void** )malloc( MESSAGE_QUEUE_STORE_OVERHEAD( queueSize) );
    action = act;
    if( !MessageQueueInitialize( &queue, queueStore, queueSize ) ) {
      ASSERT_TRUE( false );
    }
    KThreadCreate( &thr, KTHREAD_PARAMS( thrPar ) );
  }
protected:
  virtual void SetUp()
  {
  }

  virtual void TearDown() 
  {
    KThreadDelete( &thr );
    if ( queueStore ) {
      delete queueStore;
    }
    MessageQueueDeInitialize( &queue );
  }
  KThread thr;
  MessageQueue queue;
  void** queueStore;
  void (*action)(void* arg);
};

static void BlockableQueuePoster( void* arg )
{
  MessageQueue* pQueue = ( MessageQueue* )arg;
  if ( pQueue ) {
    uint64_t val = 0;
    do {
      val = ( uint64_t )MessageQueueDeQueue( pQueue );
      usleep( 50000 );
    }while( val != 0 );
  }
}

TEST_F( MsgQTest, TestBlockableQueue )
{
  SetupQueue( BlockableQueuePoster, 5 );
  for ( int32_t i = 0; i < 5; i++ ) {
    bool retval = MessageQueueEnQueue( &queue, (void*) i );
    ASSERT_TRUE( retval );
  }
}

TEST_F( MsgQTest, TestQueueingOrderIsCorrect )
{
  SetupQueue( BlockableQueuePoster, 5 );
  for( uint32_t i = 0; i < 5; i++ ) {
    bool retval = MessageQueueEnQueue( &queue, (void*) i );
    ASSERT_TRUE( retval );
  }
  for( uint32_t i = 0; i < 5; i++ ) {
    uint32_t val = ( uint32_t ) MessageQueueDeQueue( &queue );
    ASSERT_TRUE( val == i );
  }
}
