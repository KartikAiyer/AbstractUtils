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
#include "MessageThread.h"
#include <assert.h>
#include "Logable.h"
#include "Pool.h"
#include "MessageQueue.h"
#include "ThreadInterface.h"
#include "SemaphoreInterface.h"

typedef struct _MessageThread
{
  const char *threadName;
  KThreadHandle threadHandle;
  void* pPrivateData;
  bool keepRunning;
  MessageThreadInit fnInit;
  MessageThreadProcess fnProcess;
  MemPool pool;
  MessageQueue messageQ;
  uint32_t messageSize;
  KSemaHandle semaId; 
}MessageThread;

typedef struct _MessageThreadPool
{
  MessageThread threads[ MESSAGE_THREADS_MAX ];
  uint32_t poolActData[ ADDITIONAL_POOL_OVERHEAD_IN_ULONG( MESSAGE_THREADS_MAX ) ];
  MemPool threadPool;
  Logable log;
}MessageThreadPool;

static MessageThreadPool s_threadPool = { .log = { .prefix = "MsgThrPool", .enabled = true } };

#define MSG_POOL_LOG( str, ... )      Log( &s_threadPool.log, str, ##__VA_ARGS__ )

static void Thread( void *arg );

void MessageThreadSystemInit( void )
{
  if ( !PoolCreate( &s_threadPool.threadPool, (uint8_t* )s_threadPool.threads, sizeof(s_threadPool.threads ), MESSAGE_THREADS_MAX, s_threadPool.poolActData ) ) {
    assert( 0 );
  }
}

MessageThreadHandle MessageThreadCreate( const MessageThreadDef *pThreadParams )
{
  assert( pThreadParams );
  MessageThreadHandle retval = NULL;
  MessageThread *pThread = NULL;

  //TODO: These two lines should be atomic
  pThread = ( MessageThread* )PoolAlloc( &s_threadPool.threadPool );
  if ( pThread ) {

    pThread->semaId = KSemaCreate( pThreadParams->threadName, 0 );
    if ( pThread->semaId ) {
      pThread->threadName = pThreadParams->threadName;
      pThread->messageSize = pThreadParams->messageSize;
      pThread->fnInit = pThreadParams->fnInit;
      pThread->fnProcess = pThreadParams->fnProcess;
      pThread->pPrivateData = pThreadParams->pPrivateData;
      pThread->keepRunning = true;
      assert( pThread->fnInit && pThread->fnProcess );

      if( MessageQueueInitialize( &pThread->messageQ ) )
      {
        if( PoolCreate( &pThread->pool, 
                    pThreadParams->messageBackingStore, 
                    pThreadParams->messageSize * pThreadParams->messageQDepth, 
                    pThreadParams->messageQDepth, pThreadParams->pAdditionalOverhead ) ) {

          pThread->threadHandle = KThreadCreate( Thread, pThread, pThreadParams->stackSize, pThreadParams->priority );
          if ( pThread->threadHandle ) {
            //Will block till initialization of thread is complete. 
            KSemaGet( pThread->semaId, WAIT_FOREVER );
            KSemaDelete( pThread->semaId );
            pThread->semaId = 0;
            retval = ( MessageThreadHandle )pThread;
          }
          else {
            MSG_POOL_LOG( "%s(): Couldn't create Thread", __FUNCTION__ );
            PoolRelease( &pThread->pool );
            MessageQueueDeInitialize( &pThread->messageQ );
            KSemaDelete( pThread->semaId );
            PoolFree( &s_threadPool.threadPool, pThread );
          }
        }
        else {
          MSG_POOL_LOG( "%s(): Cannot Create Pool", __FUNCTION__ );
          MessageQueueDeInitialize( &pThread->messageQ );
          KSemaDelete( pThread->semaId );
          PoolFree( &s_threadPool.threadPool, pThread );
        }
      }
      else {
        MSG_POOL_LOG( "%s(): Cannot Create Message Queue", __FUNCTION__ );
        KSemaDelete( pThread->semaId );
        PoolFree( &s_threadPool.threadPool, pThread );
      }
    }
    else {
      MSG_POOL_LOG( "%s(): Couldn't Create Thread Semaphore", __FUNCTION__ );
      PoolFree( &s_threadPool.threadPool, pThread );
    }
  }
  else {
    MSG_POOL_LOG( "%s(): Reached Message Thread Maximum count", __FUNCTION__ );
  }
  return retval;
}

void* MessageThreadGetPrivateData( MessageThreadHandle hThread ) 
{
  MessageThread *pThread = ( MessageThread * )hThread;
  return pThread->pPrivateData;
}

MessageHandle MessageThreadAllocateMessage( MessageThreadHandle hThread )
{
  MessageThread *pThread = ( MessageThread * )hThread;
  MessageHandle retval = NULL;
  retval = ( MessageHandle )PoolAlloc( &pThread->pool );
  if( !retval  ) {
    MSG_POOL_LOG( "%s: Couldn't allocate message", __FUNCTION__ );
    assert( 0 );
  }
  return retval;
}

void MessageThreadDestroyMessage( MessageThreadHandle hThread, MessageHandle* phMessage )
{
  MessageThread *pThread = ( MessageThread* )hThread;
  if( phMessage && *phMessage ) {
    PoolFree( &pThread->pool, *phMessage );
    *phMessage = NULL;
  }
}

bool MessageThreadPost( MessageThreadHandle hThread, MessageHandle hMessage )
{
  bool retval = true;
  MessageThread *pThread = ( MessageThread* )hThread;
  if ( !MessageQueueEnQueue( &pThread->messageQ, hMessage ) ) {
    MSG_POOL_LOG( "Couldn't post message onto Q." );
    //For now these calls shouldn't fail
    assert( 0 );
    retval = false;
  }
  return retval;
}

static void Thread( void *arg )
{
  MessageThread *pThread = ( MessageThread* )arg;
  assert( pThread );
  //Call private Init
  pThread->fnInit( arg );
  assert( pThread->semaId );
  KSemaPut( pThread->semaId );
  while( pThread->keepRunning ) {
    void* pMsg = MessageQueueDeQueue( &pThread->messageQ );
    if( pMsg ){
      pThread->fnProcess( arg, pMsg );
      //Delete the message
      MessageThreadDestroyMessage( arg, &pMsg );
    }
    else{
      MSG_POOL_LOG( "Couldn't pull message of Q" );
      assert( 0 );
    }
  }
}