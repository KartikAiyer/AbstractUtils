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
#ifndef __MESSAGE_THREAD_IMPL_H__
#define __MESSAGE_THREAD_IMPL_H__

#include "MessageQueue.h"

/**
 * Clients using the message thread should provide additional 
 * storage space needed for maintaining the message Queue. The 
 * amount of storage is proportional to the depth of the queue. 
 * This macro provides a compile time method of determining the 
 * amount of overhead and can be used to create the necessary 
 * static storage. The storage requirements are as follows, the 
 * actual events that will be allocated and posted, the list of 
 * pointers to the posted events and a Pool Allocation Flag 
 * which is used by the message pool to keep track of free 
 * messages. 
 */
#define MESSAGE_THREAD_BACKING_STORE_SIZE( msgCount, msgType )\
  ( ( ( msgCount ) * sizeof( ( msgType ) ) ) +\
  ADDITIONAL_POOL_OVERHEAD( ( msgCount ) ) + MESSAGE_QUEUE_STORE_OVERHEAD( ( msgCount ) ) )

#endif // __MESSAGE_THREAD_IMPL_H__
