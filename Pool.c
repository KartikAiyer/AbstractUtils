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

#include "Pool.h"
#include <assert.h>
#include <string.h>
#include "Logable.h"

static Logable s_poolLog = { .prefix = "Pool", .enabled = false };

#undef LOG
#define LOG( str, ... )   Log( &s_poolLog, str, ##__VA_ARGS__ )
bool PoolCreate( MemPool* pPool, uint8_t* pBackingBuffer, uint32_t backingBufferSize, uint32_t numUnits )
{
  bool retval = false;
  if ( pPool && pBackingBuffer ) {
    if ( backingBufferSize % numUnits == 0 ) {
      pPool->pBackingStore = pBackingBuffer;
      pPool->numOfUnits = numUnits;
      pPool->pHead = 0;
      pPool->mutex = KMutexCreate( "PoolMutex" );
      if ( pPool->mutex ) {
        uint32_t i;
        uint32_t jumpBy = backingBufferSize / numUnits;
        LOG( "Adding Buffer of size %d to pool, pool sizse: %d", jumpBy, backingBufferSize );
        for ( i = 0; i < numUnits; i++ ) {
          KLIST_HEAD_PREPEND( pPool->pHead, pBackingBuffer + ( i * jumpBy ) );
          LOG( "%p", pBackingBuffer + ( i * jumpBy ) );
        }
        retval = true;
      }
      else {
        LOG( "Couldn't Initialize Mutex" );
      }
    }
    else
    {
      LOG( "Can't cleanly allocate %d units from %d bytes", numUnits, backingBufferSize );
    }
  }
  return retval;
}

void PoolRelease( MemPool* pPool )
{
  if ( pPool ) {
    bool result = KMutexLock( &pPool->mutex, WAIT_FOREVER );
    if ( result ) {
      KMutexDelete( pPool->mutex );
      pPool->mutex = 0;
      memset( pPool->pBackingStore, 0, pPool->backingBufferSize );
      pPool->pHead = 0;
      pPool->pBackingStore = 0;
    }
    else{
      LOG( "%s(): Unable to lock mutex for release", __FUNCTION__ );
      assert( 0 );
    }
  }
}

void* PoolAlloc( MemPool* pPool )
{
  void* retval = 0;
  if ( pPool ) {
    if ( KMutexLock( pPool->mutex, WAIT_FOREVER ) ) {
      KLIST_HEAD_POP( pPool->pHead, retval );
      LOG( "%s(): Retval: %p", __FUNCTION__, retval );
      KMutexUnlock( pPool->mutex );
    }
    else
    {
      LOG( "%s(): Couldn't lock mutex", __FUNCTION__ );
      assert( 0 );
    }
  }
  return retval;
}

void PoolFree( MemPool* pPool, void* buf )
{
  if ( pPool && buf ) {
    KListElem *pElem = (KListElem* )buf;
    LOG( "Freeing: %p", pElem );
    if ( pElem->next != 0 || pElem->prev != 0 ) {
      LOG( "%s(): !!! CRITICAL ERROR. BUFFER TO FREE DOES NOT BELONG TO POOL", __FUNCTION__ );
      assert( 0 );
    }
    if ( KMutexLock( &pPool->mutex, WAIT_FOREVER ) ) {
      KLIST_HEAD_PREPEND( pPool->pHead, buf );
      KMutexUnlock( &pPool->mutex );
    }
    else
    {
      LOG( "%s(): Couldn't lock mutex", __FUNCTION__ );
      assert( 0 );
    }
  }
}
