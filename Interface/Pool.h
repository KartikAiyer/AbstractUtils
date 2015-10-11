/**
 * Confidential!!!
 * Source code property of Blue Clover Design LLC.
 *
 * Demonstration, distribution, replication, or other use of the
 * source codes is NOT permitted without prior written consent
 * from Blue Clover Design.
 */
#ifndef __POOL_H__
#define __POOL_H__


#include <limits.h>
#include "MutexInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MemoryPools - A fixed Block mempool api.
 *  Typically we don't want to rely on malloc and free due to
 *  fragmentation issues. This api allows you to pass in a
 *  static data storage and provides routines to allocate /
 *  release fixed sized chunks from the static backing store
 *  provided. In addition, the client also provides the storage
 *  needed to keep track of free buffers. Free buffers are kept
 *  track off using bit masks. The minimum size of the bitmask
 *  is a uin32_t. You need to provide as many uint32_t units to
 *  keep track of the num of units you desire to allocate from
 *  the pool. For instance, if you have 32 units total, you need
 *  to provide one byte which can keep track of upto 32 units.
 */ 

/**
 * @struct MemPool - Structure holding Pool information.
 * Clients will own an instance of this and use the
 * corresponding routines to Init/DeInit and use the pool.
 */
typedef struct _MemPool
{
  void* pBackingStore;
  uint32_t* pFreeBits;
  uint32_t backingBufferSize;
  uint32_t numOfUnits;
  KMutex mutex;
}MemPool;

#define CEIL_DIV( a, b )    ( (a) % (b) ) ? ( ( (a) / (b) ) + 1 ) : ( (a) / (b) )

#define ADDITIONAL_POOL_OVERHEAD( totalAllocationUnits )\
  ( CEIL_DIV( ( totalAllocationUnits ), CHAR_BIT * sizeof( uint32_t ) ) * sizeof( uint32_t ) )
#define POOL_STORE_SIZE( totalAllocationUnits, sizeOfUnit )\
  ( (sizeOfUnit)*(totalAllocationUnits) + ADDITIONAL_POOL_OVERHEAD( (totalAllocationUnits) ) )
/**
 * PoolCreate - Creates a pool from a provided Backing store. 
 * The user provides a backing buffer and the number of fixed 
 * size units to be able to allocate from it. If the num of 
 * total units to allocate does not cleanly divide the 
 * ( backingBufferSize - poolOverhead), the pool creation will fail. In addition
 * you also need to provide the storage required to maintain the
 * free list. This is done by using a bit mask. A minimum of 1 
 * uint32_t is needed ( upto 32 items ). To determine the additional overhead
 * that you must provide to your backing store use ADDITIONAL_POOL_OVERHEAD()
 * example:
 * Assume you have a data structure DataStruct_t and you want to allocate TOTAL_NUM_OF_UNITS.
 * provide a backing buffer as such
 * - uint8_t dataStructBackingStore[ ( sizeof( DataStruct_t ) * TOTAL_NUM_OF_UNITS )+
 *                                   ADDITIONAL_POOL_OVERHEAD( TOTAL_NUM_OF_UNITS ) ];
 * Specify the backing store size as
 * sizeof(dataStructBackingStore). 
 * 
 * 
 * @param pPool -  pointer to the pool to initialize
 * @param backingBuffer - backing buffer to allocate from. 
 * @param backingBufferSize - backing buffer size.
 * @param numOfUnits - total number of units to allocate. Must 
 *                   completely divide backingBufferSize - ADDITIONAL_POOL_OVERHEAD( numOfUnits ).
 *
 * @return bool - true if successfully created. 
 */
bool PoolCreate( MemPool* pPool,
                 uint8_t* backingBuffer,
                 uint32_t backingBufferSize,
                 uint32_t numOfUnits );

/**
 * PoolRelease - Release a memory Pool. Allocations and Free 
 * operation will fail after a pool has been released. 
 * 
 * 
 * @param pPool - Pointer to pool to release. 
 */
void PoolRelease( MemPool* pPool );

/**
 * PoolAlloc - Allocates a data unit from the Pool.
 * 
 * 
 * @param pPool - Pool to allocate from. 
 * 
 * @return void* - NULL if allocation failed. 
 */
void* PoolAlloc( MemPool* pPool );

/**
 * PoolFree - Reeleases a buffer back to the pool. ONLY FREE 
 * BUFFERS FROM THE SAME POOL. USE RESPONSIBLY 
 * 
 * 
 * @param pPool - pool to free from. 
 * @param buf - buffer to free. 
 */
void PoolFree( MemPool* pPool, void* buf );

#ifdef __cplusplus
}
#endif

#endif //__POOL_H__
