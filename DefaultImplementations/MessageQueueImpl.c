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

#include <MessageQueue.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

bool MessageQueueInitialize( MessageQueue* pQueue, void** pQueueStore, uint32_t queueSize )
{
  bool retval = false;
  if ( pQueue && pQueueStore && queueSize > 0 ) {
    if( KMutexCreate( &pQueue->mutex, "Queue Mutex" ) &&
        KSemaCreate( &pQueue->fullSema, "Queue Full Sema", queueSize ) &&
        KSemaCreate( &pQueue->emptySema, "Queue Empty Sema", 0 ) ) {
      pQueue->arrayQueueOfItems = pQueueStore;
      pQueue->head = pQueue->tail = 0;
      pQueue->size = queueSize;   
      pQueue->isInitialized = true;
      retval = true;
    }
  }        
  return retval;
}

void MessageQueueDeInitialize( MessageQueue* pQueue )
{
  if ( pQueue ) {
    KMutexDelete( &pQueue->mutex );
    KSemaDelete( &pQueue->fullSema );
    KSemaDelete( &pQueue->emptySema );
    pQueue->arrayQueueOfItems = 0;
    pQueue->head = pQueue->tail = pQueue->size = 0;
    pQueue->isInitialized = false;
  }
}

bool MessageQueueEnQueue( MessageQueue* pQueue, void *pItem )
{
  bool retval = false;
  if ( pQueue && pQueue->isInitialized ) {
    if( KSemaGet ( &pQueue->fullSema, WAIT_FOREVER ) ) {
      if ( KMutexLock( &pQueue->mutex, WAIT_FOREVER ) ) {
        KSemaPut( &pQueue->emptySema );
        pQueue->arrayQueueOfItems[ pQueue->head ] = pItem;
        pQueue->head = ( pQueue->head + 1 ) % pQueue->size;
        KMutexUnlock( &pQueue->mutex );
        retval = true;
      } else {
        LOG( "%s(): Could'n't Get Queue Mutex", __FUNCTION__ );
      }
    }
    else {
      LOG( "%s(): Unable to Get Full Semaphore", __FUNCTION__ );
    }
  } else {
    LOG( "%s(): Invalid Queue( %p ) or not init", __FUNCTION__, pQueue );
  }
  return retval;
}

void* MessageQueueDeQueue( MessageQueue* pQueue )
{
  void* retval = 0;
  if ( pQueue && pQueue->isInitialized ) {
    if( KSemaGet( &pQueue->emptySema, WAIT_FOREVER ) ) {
      if ( KMutexLock( &pQueue->mutex, WAIT_FOREVER ) ) {
        KSemaPut( &pQueue->fullSema );
        assert( pQueue->arrayQueueOfItems[ pQueue->tail ] );
        retval = pQueue->arrayQueueOfItems[ pQueue->tail ];
        pQueue->tail = ( pQueue->tail + 1 ) % pQueue->size;
        KMutexUnlock( &pQueue->mutex );
      } else {
        LOG( "%s(): Coulnd't Get Queue Mutex", __FUNCTION__ );
      }
    } else {
      LOG( "%s(): Unable to Get Empty Semaphore", __FUNCTION__ );
    }
  }
  return retval;
}

#ifdef __cplusplus
}
#endif
