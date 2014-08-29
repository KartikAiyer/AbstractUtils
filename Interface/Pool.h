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


#include "Klist.h"
#include "wiced.h"

/** @defgroup MemoryPools - A fixed Block mempool api.
 *  Typically we don't want to rely on malloc and free due to
 *  fragmentation issues. This api allows you to pass in a
 *  static data storage and provides routines to allocatei /
 *  release fixed sized chunks from the static backing store
 *  provicded.
 */ 

/**
 * @struct MemPool - Structure holding Pool information.
 * Clients will own an instance of this and use the
 * corresponding routines to Init/DeInit and use the pool.
 */
typedef struct _MemPool
{
  void* pBackingStore;
  uint32_t backingBufferSize;
  uint32_t numOfUnits;
  KListHead *pHead;
  wiced_mutex_t mutex;
}MemPool;

/**
 * PoolCreate - Creates a pool from a provided Backing store. 
 * The user provides a backing buffer and the number of fixed 
 * size units to be able to allocate from it. If the num of 
 * total units to allocate does not cleanly divide the 
 * backingBufferSize, the pool creation will fail. If you want 
 * to allocate a bunch of DataStruct_t then create a packing 
 * store - DataStruct_t 
 * dataStructBackingStore[TOTAL_NUM_OF_UNITS] and specify the 
 * backing store size as sizeof(dataStructBackingStore) 
 * 
 * 
 * @param pPool -  pointer to the pool to initialize
 * @param backingBuffer - backing buffer to allocate from. 
 * @param backingBufferSize - backing buffer size.
 * @param numOfUnits - total number of units to allocate. Must 
 *                   completely divide backingBufferSize.
 * 
 * @return bool - true if successfully created. 
 */
bool PoolCreate( MemPool* pPool, uint8_t* backingBuffer, uint32_t backingBufferSize, uint32_t numOfUnits );
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
#endif //__POOL_H__
