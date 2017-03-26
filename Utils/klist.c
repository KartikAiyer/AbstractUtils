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
