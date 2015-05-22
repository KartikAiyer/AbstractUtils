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
#include <limits.h>
#include "Logable.h"

#ifdef __cplusplus
extern "C" {
#endif

static Logable s_poolLog = { .prefix = "Pool", .enabled = false };

#undef LOG
#define LOG( str, ... )   Log( &s_poolLog, str, ##__VA_ARGS__ )


bool PoolCreate( MemPool* pPool, uint8_t* pBackingBuffer, uint32_t backingBufferSize, uint32_t numUnits, uint32_t* pFreeBits )
{
  bool retval = false;
  if ( pPool && pBackingBuffer && pFreeBits ) {
    if ( backingBufferSize % numUnits == 0 ) {
      uint32_t i = 0;
      pPool->pBackingStore = pBackingBuffer;
      pPool->numOfUnits = numUnits;
      pPool->pFreeBits = pFreeBits;
      for ( i = 0; i < CEIL_DIV( numUnits, CHAR_BIT * sizeof( uint32_t ) ); i++ ) {
        *( pFreeBits + i ) = (uint32_t)-1;
      }
      
      if ( KMutexCreate( &pPool->mutex, "PoolMutex" ) ) {
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
      KMutexDelete( &pPool->mutex );
      memset( pPool->pBackingStore, 0, pPool->backingBufferSize );
      pPool->pBackingStore = 0;
      pPool->pFreeBits = 0;
    }
    else{
      LOG( "%s(): Unable to lock mutex for release", __FUNCTION__ );
      assert( 0 );
    }
  }
}

static uint32_t GetFreeIndex( MemPool* pPool, uint32_t levelDeep )
{
  if ( levelDeep < CEIL_DIV( pPool->numOfUnits, sizeof( uint32_t ) * CHAR_BIT ) ) {
    uint32_t bitField = *( pPool->pFreeBits + levelDeep );
    uint32_t freeLocation = __builtin_ctz( bitField );
    if ( freeLocation < 32 ) {
      return ( levelDeep * ( CHAR_BIT * sizeof( uint32_t ) ) + freeLocation );
    }
    else {
      return GetFreeIndex( pPool, levelDeep + 1 );
    }
  }
  else 
    return levelDeep * ( CHAR_BIT * sizeof( uint32_t ) );
}

static void MarkIndexFree( MemPool* pPool, uint32_t index, uint32_t levelDeep )
{
  if ( index < pPool->numOfUnits ) {
    if ( index < CHAR_BIT * sizeof( uint32_t ) ) {
      uint32_t* pBits = pPool->pFreeBits + levelDeep;
      LOG( "%s(): Old: %p",__FUNCTION__, *pBits );
      *pBits &= ( 1 << index );
      LOG( "%s(): New: %p", __FUNCTION__, *pBits );
    }
    else {
      MarkIndexFree( pPool, index - ( CHAR_BIT * sizeof( uint32_t ) ), levelDeep++ );
    }
  }
  else {
    LOG( "%s(): Invalid Index to free: %d, Total: %d", __FUNCTION__, index, pPool->numOfUnits );
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
        LOG( "%s(): Retval: %p ( index: %d )", __FUNCTION__, retval, freeIndex );
      }
      KMutexUnlock( &pPool->mutex );
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
    uint32_t indexToFree = ( uint32_t )( (uint8_t*)buf - (uint8_t*)pPool->pBackingStore );
    if ( indexToFree >= pPool->numOfUnits ) {
      LOG( "%s(): Got out of bounds index to free: %d, ptr: %p (start: %p)", 
           __FUNCTION__, indexToFree, buf, pPool->pBackingStore );
      assert( 0 );
    }
    if ( KMutexLock( &pPool->mutex, WAIT_FOREVER ) ) {
      MarkIndexFree( pPool, indexToFree, 0 );
      KMutexUnlock( &pPool->mutex );
    }
    else
    {
      LOG( "%s(): Couldn't lock mutex", __FUNCTION__ );
      assert( 0 );
    }
  }
}

#ifdef __cplusplus
}
#endif
