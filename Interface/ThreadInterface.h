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
#ifndef __THREAD_INTERFACE_H__
#define __THREAD_INTERFACE_H__

#include <InterfacePrivateCommon.h>
#include <PlatformInterface.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint32_t KThreadHandleSize;

typedef struct _KThreadCreateParams
{
  KThreadCallback fn;
  const char* pThreadName;
  void* threadArg;
  uint32_t stackSizeInBytes;
  uint32_t threadPriority; 
}KThreadCreateParams;

#define KTHREAD_CREATE_PARAMS( varName, pName, f, arg, stkSize, priority )\
const KThreadCreateParams kthreadParams_##varName = \
{\
  .pThreadName = pName,\
  .fn = f,\
  .threadArg = arg,\
  .stackSizeInBytes = stkSize,\
  .threadPriority = priority\
}

#define KTHREAD_PARAMS( varName ) &kthreadParams_##varName

bool KThreadCreate( KThread* pThread, const KThreadCreateParams* pParams ); 
bool KThreadDelete( KThread* pThread );
bool KThreadJoin( KThread* pThread );

#ifdef __cplusplus
}
#endif
#endif

