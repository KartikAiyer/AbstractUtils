/**
 * Confidential!!!
 * Source code property of Blue Clover Design LLC.
 *
 * Demonstration, distribution, replication, or other use of the
 * source codes is NOT permitted without prior written consent
 * from Blue Clover Design.
 */


#include "Utilities/Pool.h"
#include "Utilities/Logable.h"
#include <assert.h>

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
      if ( wiced_rtos_init_mutex( &pPool->mutex ) == WICED_SUCCESS ) {
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
    wiced_result_t result; 
    result = wiced_rtos_lock_mutex( &pPool->mutex );
    if ( result == WICED_SUCCESS ) {
      wiced_rtos_deinit_mutex( &pPool->mutex );
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
    if ( wiced_rtos_lock_mutex( &pPool->mutex ) == WICED_SUCCESS ) {
      KLIST_HEAD_POP( pPool->pHead, retval );
      LOG( "%s(): Retval: %p", __FUNCTION__, retval );
      wiced_rtos_unlock_mutex( &pPool->mutex );
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
    if ( wiced_rtos_lock_mutex( &pPool->mutex ) == WICED_SUCCESS ) {
      KLIST_HEAD_PREPEND( pPool->pHead, buf );
      wiced_rtos_unlock_mutex( &pPool->mutex );
    }
    else
    {
      LOG( "%s(): Couldn't lock mutex", __FUNCTION__ );
      assert( 0 );
    }
  }
}
