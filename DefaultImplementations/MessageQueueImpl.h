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
#ifndef __MESSAGE_QUEUE_IMPL_H__
#define __MESSAGE_QUEUE_IMPL_H__

#include "MutexInterface.h"
#include "SemaphoreInterface.h"
#include "Logable.h"

typedef struct _MessageQueue
{
  KMutex mutex;
  KSema fullSema;
  KSema emptySema;
  void** arrayQueueOfItems;
  uint32_t head, tail, size;
  bool isInitialized;
}MessageQueue;

#define MESSAGE_QUEUE_STORE_OVERHEAD( queueSize ) ( sizeof( void* ) * ( queueSize ) )
#define MESSAGE_QUEUE_DEF( name, maxSize )  \
  void* msgQueueDataStore_##name[ maxSize ];\
  MessageQueue msgQueue_##name

#define MESSAGE_QUEUE( name ) msgQueue_##name
#define MESSAGE_QUEUE_STORE( name ) msgQueueDataStore_##name
#endif //  __MESSAGE_QUEUE_IMPL_H__
