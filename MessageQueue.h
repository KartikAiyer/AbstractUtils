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
#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#include "MutexInterface.h"

typedef struct _MessageQueue
{
  KMutex mutex;
  void** arrayQueueOfItems;
}MessageQueue;

#define MESSAGE_QUEUE_DEF( name, maxSize )  \
  void* msgQueueDataStore_##name[ maxSize ];\
  MessageQueue msgQueue_##name = { .arrayQueueOfItems = msgQueueDataStore_##name };

#define MESSAGE_QUEUE( name ) msgQueue_##name;

bool MessageQueueInitialize( MessageQueue* pQueue );
void MessageQueueDeInitialize( MessageQueue* pQueue );
bool MessageQueueEnQueue( MessageQueue* pQueue, void *pItem );
void* MessageQueueDeQueue( MessageQueue* pQueue );
#endif // __MESSAGE_QUEUE_H__
