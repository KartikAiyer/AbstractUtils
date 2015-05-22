/**
 * Confidential!!! 
 * Source code property of Blue Clover Design LLC.
 *  
 * Demonstration, distribution, replication, or other use of the 
 * source codes is NOT permitted without prior written consent 
 * from Blue Clover Design. 
 */

#include "klist.h"

#ifdef __cplusplus
extern "C" {
#endif 

void KQueueInit( KQueue* pQueue )
{
  if ( pQueue ) {
    pQueue->pHead = pQueue->pTail = 0;
  }
}

bool KQueueIsEmpty( KQueue *pQueue )
{
  bool retval = true;
  if ( pQueue && pQueue->pHead != 0 && pQueue->pTail != 0 ) {
    retval = false;
  }
  return retval;
}

void KQueueInsert( KQueue *pQueue, KListElem *pElem )
{
  if ( pQueue && pElem ) {
    KLIST_TAIL_APPEND( pQueue->pTail, pElem );
    if ( pQueue->pHead == 0 ) {
      pQueue->pHead = pQueue->pTail;
    }
  }
}

KListElem* KQueueDequeue( KQueue *pQueue )
{
  KListElem *pRetVal = 0;
  if ( !KQueueIsEmpty( pQueue ) ) {
    if ( pQueue->pHead == pQueue->pTail ) {
      //Only one element
      pQueue->pTail = 0;
    }
    pRetVal = pQueue->pHead;
    pQueue->pHead = pRetVal->next;
    KLIST_REMOVE_ELEM( pRetVal );
  }
  return pRetVal;
}

#ifdef __cplusplus
}
#endif 
