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
#ifndef __MESSAGE_THREAD_H__
#define __MESSAGE_THREAD_H__

#include <stdint.h>
#include <stdbool.h>
#include "klist.h"
#include "Pool.h"
#include "MessageThreadImpl.h"


#define MESSAGE_THREADS_MAX            ( 5 )

typedef void* MessageHandle;
typedef const void* MessageThreadHandle;

/**
 * The function callback that is called to initialize the thraed 
 * when the thread is started. MessageThreadCreate will not 
 * return till this function is completed. 
 *  
 * @param MessageThreadHandle - Handle to the message thread. 
 */
typedef void (*MessageThreadInit)( MessageThreadHandle hThread );

/**
 * This callback is called when a message is received in the 
 * internal message queue. 
 *  
 * @param hThread - Handle to the mesage thread. 
 * @param hMessage - Handle to the message recd. 
 */
typedef void (*MessageThreadProcess)( MessageThreadHandle hThread, MessageHandle hMessage );

/**
 * @struct MessageThreadDef 
 * @brief - Message thread initialization structure 
 *  
 * Need to specify, stack size, priority, number of entries that 
 * can be held in the message Q, size of the messages, size of 
 * the pool from which messages are allocated and any private 
 * data. 
 */
typedef struct _MessageThreadDef
{
  const char* threadName;   /**< Name of thread. A statically allocated name is best. */
  uint32_t stackSize;       /**< Stack size in bytes */
  uint32_t priority;
  uint8_t* messageBackingStore;
  uint32_t messageQDepth;   /**< The number of messages that can Q up in the thread  */
  uint32_t messageSize;     /**< The size of each message processed by the thread */
  void *pPrivateData;       /**< Private data that is passed to the thread functions. Holds thread state. */
  MessageThreadInit fnInit; /**< Thread Initialization function */
  MessageThreadProcess fnProcess; /**< Function to process each incoming message */
}MessageThreadDef;

/**
 * A Convenience method used to create a MessageThreadDef to 
 * be used with message Thread Create. 
 */
#define MESSAGE_THREAD_DEF( name, stkSz, pri, msgBackStore, qDepth, msgType, priv, init, process )\
const MessageThreadDef messageThreadDef_##name =\
{\
  .threadName = #name,\
  .stackSize = stkSz,\
  .priority = pri,\
  .messageBackingStore = msgBackStore,\
  .messageQDepth = qDepth,\
  .messageSize = sizeof(msgType),\
  .pPrivateData = priv,\
  .fnInit = init,\
  .fnProcess = process\
}

#define MESSAGE_THREAD( name ) &messageThreadDef_##name

/**
 * Used to create a message thread. It is not safe to post 
 * events to the message thread till this function returns. The 
 * function will not return till the MessageThreadInit function 
 * supplied by the client instance completes.  
 * 
 * 
 * @param pThreadDef: const MessageThreadDef* - Message thread 
 *                  initialization params.
 * 
 * @return MessageThreadHandle - Handle to the message thread.
 */
MessageThreadHandle MessageThreadCreate( const MessageThreadDef *pThreadDef );

void MessageThreadDestroy( MessageThreadHandle hThread );
/**
 * Used to get a pointer to the private data that was supplied 
 * to the message thread during creation. 
 * 
 * 
 * @param hThread: MessageThreadHandle - Handle to message 
 *               thread.
 * 
 * @return void* - pointer to private data if any. 
 */
void* MessageThreadGetPrivateData( MessageThreadHandle hThread );

/**
 * Allcoate a message that will be used to post to the message 
 * thread. Cliednt should ideally make convenience routines that 
 * combine MessageThreadAllocateMessage() and 
 * MessageThreadPost(). 
 * 
 * 
 * @param hThread: MessageThreadHandle - Handle to message 
 *               thread.
 * 
 * @return MessageHandle - Handle to message
 */
MessageHandle MessageThreadAllocateMessage( MessageThreadHandle hThread );

/**
 * Used to destroy a message. External clients don't have to 
 * call this function if the messsage is posted to the thread. 
 * The thread will automatically delete the message after the 
 * Process Callback is called on the message. 
 * 
 * 
 * @param hThread: MessageThreadHandle - handle to thread
 * @param phMessage: MessageHandle* - pointer to messsage 
 *                 handle.
 */
void MessageThreadDestroyMessage( MessageThreadHandle hThread, MessageHandle* phMessage );

/**
 * Used to post messages to the message thread. Clients should 
 * combine this with MessageThreadAllocate to make a convenience 
 * routine to post events to their threads. 
 * 
 * 
 * @param hThread: MessageThreadHandle - Handle to message 
 *               thread
 * @param hMessage: MessageHandle - Handle to message
 * 
 * @return bool - True if the event has been posted.
 */
bool MessageThreadPost( MessageThreadHandle hThread, MessageHandle hMessage );

#endif // __MESSAGE_THREAD_H__