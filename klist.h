/**
 * Confidential!!! 
 * Source code property of Blue Clover Design LLC.
 *  
 * Demonstration, distribution, replication, or other use of the 
 * source codes is NOT permitted without prior written consent 
 * from Blue Clover Design. 
 */
#ifndef __KLIST_H__
#define __KLIST_H__

#include <stdbool.h>
#include <stdint.h>


/** @defgroup klist - a quick linked list implementation
 *  This is a quick macro based Linked List implementation
 *  The idea heare is to represent a linked list by the head
 *  of the list. Every element that will be added to the list
 *  must have as its first member the @ref KListElem data
 *  structure. The quickest way to add an element to the list
 *  is to prepend it to the head of the list ( thus modifying
 *  the head). Similarly the quickes pop, is to pop the head
 *  of the list.
 **/ 

/**
 * @struct KListElem - Sentinel for all elements that wish to be 
 *         added to a list.
 *  
 * This holds the next and prev pointers. All data structures 
 * that need to be added to a list must have this as its first 
 * element. Thus if you want to put some arbitrary data into a 
 * KList, you need to encapsulate it in a data structure with 
 * KListElem being its first member and then use instances of 
 * that structure to insert. 
 */
typedef struct _KListElem
{
  struct _KListElem *prev;
  struct _KListElem *next;
}KListElem, KListHead, KListTail;

/**
 * @struct KQueue - a Queue implementation based on KList. 
 * This is not thread safe. 
 */
typedef struct _KQueue
{
  KListHead *pHead;
  KListTail *pTail;
}KQueue;

/**
 * Callback that is executed on every element of a List when 
 * called using KLIST_HEAD_FOREACH 
 */
typedef void (*KListForEachCb)( KListElem *pElem, ... );  

#define KLIST_HEAD_INIT( pHead )     do { if( (pHead) ){ (pHead)->prev = (pHead)->next = 0; } }while(0)
#define KLIST_HEAD_FOREACH( pHead, fnHandler, handlerArgs... )  do {\
                                                  KListElem *pElem = (pHead);\
                                                  while( pElem ) {\
                                                    fnHandler( pElem, ##handlerArgs );\
                                                    pElem = pElem->next;\
                                                  }\
                                                }while( 0 )

#define KLIST_HEAD_PREPEND( pHead, pItem ) do { \
                                              if( (pItem) ) {\
                                                if( (pHead) ) {\
                                                  (pHead)->prev = ( KListElem* )(pItem);\
                                                  ( ( KListElem* ) (pItem) )->prev = 0;\
                                                  ( ( KListElem* ) (pItem) )->next = (pHead);\
                                                }\
                                                (pHead) = ( KListHead *)(pItem);\
                                              }\
                                             }while( 0 )
#define KLIST_HEAD_POP( pHead, pItem )    do { \
                                             if( (pHead) ) { \
                                               KListElem** ppElem = ( ( KListElem** ) &(pItem) );\
                                               *ppElem = (pHead);\
                                               (pHead) = (pHead)->next;\
                                               ( *ppElem )->next = ( *ppElem )->prev = 0;\
                                             }\
                                             else (pItem) = 0;\
                                          }while( 0 )
#define KLIST_TAIL_INIT( pTail )      do { if( (pTail) ) { (pTail)->prev = (pTail)->next = 0; } }while( 0 )

#define KLIST_TAIL_APPEND( pTail, pItem )  do{\
                                             if( (pItem) ) {\
                                               if( (pTail) ) {\
                                                 (pTail)->next = ( KListElem* )(pItem);\
                                                 ( ( KListElem* ) (pItem) )->next = 0;\
                                                 ( ( KListElem* ) (pItem) )->prev = (pTail);\
                                               }\
                                               (pTail) = ( KListTail* )(pItem);\
                                             }\
                                           }while( 0 )

#define KLIST_REMOVE_ELEM( pElem )   do { \
                                        KListElem *pEl = ( KListElem * )(pElem);\
                                        if( pEl ){\
                                          if( pEl->next ) { pEl->next->prev = pEl->prev; }\
                                          if( pEl->prev ) { pEl->prev->next = pEl->next; }\
                                          pEl->next = pEl->prev = 0;\
                                        }\
                                     }while( 0 )

/**
 * KQueueInit - Initializes the Queue.
 * 
 * 
 * @param pQueue - Allocated Queue owned by client.
 */
void KQueueInit( KQueue* pQueue );

/**
 * KQueueIsEmpty - Queries if queue is empty
 * 
 * 
 * @param pQueue - Allocated and initialized Queue. 
 * 
 * @return bool - true if empty. 
 */
bool KQueueIsEmpty( KQueue *pQueue );

/**
 * KQueueInsert - Inserts an element into the tail of the queue.
 * 
 * 
 * @param pQueue - Allocated and initialized queue.
 * @param pElem - element to insert. 
 */
void KQueueInsert( KQueue *pQueue, KListElem *pElem );

/**
 * KQueueDequeue - Removes and element from the head of the 
 * queue. 
 * 
 * 
 * @param pQueue - ALlocated and initialized queue. 
 * 
 * @return KListElem* - Element that has been dequeued. 
 */
KListElem* KQueueDequeue( KQueue *pQueue );

#endif //__KLIST_H__
