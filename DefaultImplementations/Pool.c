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

#include <Pool.h>
#include <assert.h>
#include <string.h>
#include <miscutils.h>
#include <ConsoleLog.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef POOL_LOG
#ifdef CONFIG_POOL_ALLOCATION_LOGS
#define POOL_LOG( str, ... )   ConsoleLogLine( str, ##__VA_ARGS__ )
#else
#define POOL_LOG( str, ... )
#endif

#define FREE_BITMASK_SIZE_IN_ULONG(numUnits)    CEIL_DIV( numUnits, SINGLE_BITMASK_CAPACITY )

bool PoolCreate( MemPool* pPool,
                 uint8_t* pBackingBuffer,
                 uint32_t backingBufferSize,
                 uint32_t numUnits )
{
  bool retval = false;
  if ( pPool && pBackingBuffer ) {
    uint32_t i = 0;
    uint32_t actualBackingBufferSize = backingBufferSize - ADDITIONAL_POOL_OVERHEAD( numUnits );

    pPool->pBackingStore = pBackingBuffer;
    pPool->backingBufferSize = backingBufferSize;
    pPool->numOfUnits = numUnits;
    pPool->pFreeBits = (uint32_t*) ( pBackingBuffer + actualBackingBufferSize );
    if( actualBackingBufferSize % numUnits == 0 ) {
      for( i = 0; i < FREE_BITMASK_SIZE_IN_ULONG( numUnits ); i++ ) {
        *(pPool->pFreeBits + i) = ( uint32_t ) -1;
      }

      if( KMutexCreate( &pPool->mutex, "PoolMutex" )) {
        retval = true;
      }
      else {
        POOL_LOG( "Couldn't Initialize Mutex" );
      }
    } else {
      memset( pPool, 0, sizeof( MemPool ) );
      POOL_LOG( "PoolSize Error. Cannot Allocate %u units from buffer of size %u bytes, overhead needed: %u bytes",
            numUnits, backingBufferSize, ADDITIONAL_POOL_OVERHEAD( numUnits ) );
    }
  }
  return retval;
}

void PoolRelease( MemPool* pPool )
{
  if ( pPool ) {
    bool result = KMutexLock( &pPool->mutex, WAIT_FOREVER );
    if ( result ) {
      KMutexDelete( &pPool->mutex );
      memset( pPool->pBackingStore, 0, pPool->backingBufferSize );
      pPool->pBackingStore = 0;
      pPool->pFreeBits = 0;
    }
    else{
      POOL_LOG( "%s(): Unable to lock mutex for release", __FUNCTION__ );
      assert( 0 );
    }
  }
}

static uint32_t GetFreeIndex( MemPool* pPool, uint32_t levelDeep )
{
  if ( levelDeep < FREE_BITMASK_SIZE_IN_ULONG( pPool->numOfUnits ) ) {
    uint32_t bitField = *( pPool->pFreeBits + levelDeep );
    uint32_t freeLocation = ctz( bitField );
    if ( bitField && freeLocation < 32 ) {
      return ( levelDeep * ( SINGLE_BITMASK_CAPACITY ) + freeLocation );
    }
    else {
      return GetFreeIndex( pPool, levelDeep + 1 );
    }
  }
  else 
    return levelDeep * SINGLE_BITMASK_CAPACITY;
}

static void MarkIndex( MemPool* pPool, bool shouldFree, uint32_t index, uint32_t levelDeep )
{
  if ( index < pPool->numOfUnits ) {
    if ( index < SINGLE_BITMASK_CAPACITY ) {
      uint32_t* pBits = pPool->pFreeBits + levelDeep;
      POOL_LOG( "%s(): Old: %p",__FUNCTION__, *pBits );
      if( shouldFree ) {
        *pBits |= (1 << index);
      } else {
        *pBits &= ~( 1 << index );
      }
      POOL_LOG( "%s(): New: %p", __FUNCTION__, *pBits );
    }
    else {
      MarkIndex( pPool, shouldFree, index - SINGLE_BITMASK_CAPACITY, levelDeep + 1 );
    }
  }
  else {
    POOL_LOG( "%s(): Invalid Index to free: %d, Total: %d", __FUNCTION__, index, pPool->numOfUnits );
    assert( 0 );
  }
}

void* PoolAlloc( MemPool* pPool )
{
  void* retval = 0;
  if ( pPool ) {
    if ( KMutexLock( &pPool->mutex, WAIT_FOREVER ) ) {
      uint32_t freeIndex = GetFreeIndex( pPool, 0 );
      if ( freeIndex < pPool->numOfUnits ) {
        uint32_t sizeofUnit = pPool->backingBufferSize / pPool->numOfUnits;
        retval = ( ( uint8_t* )pPool->pBackingStore + ( sizeofUnit * freeIndex ) );
        MarkIndex( pPool, false, freeIndex, 0 );
        POOL_LOG( "%s(): Retval: %p ( index: %d )", __FUNCTION__, retval, freeIndex );
      }
      KMutexUnlock( &pPool->mutex );
    }
    else
    {
      POOL_LOG( "%s(): Couldn't lock mutex", __FUNCTION__ );
      assert( 0 );
    }
  }
  return retval;
}

void PoolFree( MemPool* pPool, void* buf )
{
  if ( pPool && buf && buf >= (void*)pPool->pBackingStore && buf < (void*)(pPool->pBackingStore + pPool->backingBufferSize) ) {
    uint32_t actualSizeOfPool = pPool->backingBufferSize - ADDITIONAL_POOL_OVERHEAD( pPool->numOfUnits );
    uint32_t unitSize = actualSizeOfPool / pPool->numOfUnits;
    uint32_t indexToFree = ( uint32_t )( (uint8_t*)buf - (uint8_t*)pPool->pBackingStore ) / unitSize ;
    if ( indexToFree >= pPool->numOfUnits ) {
      POOL_LOG( "%s(): Got out of bounds index to free: %d, ptr: %p (start: %p)", 
           __FUNCTION__, indexToFree, buf, pPool->pBackingStore );
      assert( 0 );
    }
    if ( KMutexLock( &pPool->mutex, WAIT_FOREVER ) ) {
      MarkIndex( pPool, true, indexToFree, 0 );
      KMutexUnlock( &pPool->mutex );
    }
    else
    {
      POOL_LOG( "%s(): Couldn't lock mutex", __FUNCTION__ );
      assert( 0 );
    }
  }
}

#ifdef __cplusplus
}
#endif
