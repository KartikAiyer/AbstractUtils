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

#ifndef __LOG_BUFFER_H__
#define __LOG_BUFFER_H__

#include <PlatformInterface.h>
#include <MutexInterface.h>
/**
 *  @file LogBuffer 
 *  This Ring Buffer works for 1 byte characters to be used to
 *  capture logging.
 *  
 * 
 */

typedef struct _LogBuffer
{
  char* pBuffer;
  bool isInit;
  uint32_t bufferSize;
  uint32_t head, tail;
  bool inEscapeSequence;
}LogBuffer;

/**
 * LogBufferInit - Initializes the ring buffer. 
 * 
 * \param pLb - Provide a valid Ring Buffer but don't fill it up
 * \param pBuffer - A backing store for the ring buffer
 * \param bufferSize - The total size of the backing buffer 
 */
bool LogBufferInit( LogBuffer* pLb, char* pBuffer, uint32_t bufferSize );

/**
 * LogBufferPush - This will push the string character by 
 * character into the Log Buffer. 
 * 
 * \param pLb - Pointer to intialized Log Buffer 
 * \param pString - pString. 
 * \param stringSize - string size as determined by strlen()
 * 
 * \return bool - true if successful 
 */
bool LogBufferPush( LogBuffer* pLb, char* pString, uint32_t stringSize );

/**
 * LogBufferIsEmpty - Returns true if the log buffer is empty. 
 *  
 * 
 * \param pLb 
 * 
 * \return bool 
 */
bool LogBufferIsEmpty( LogBuffer* pLb );

/**
 * LogBufferCharPop - Pops the character at the tail of the log buffer.
 * Returns true for a successful read and false if the Log buffer is empty.
 * 
 * \param pLb - pointer to valid Log Buffer
 * \param pChar - pointer to character which will contain popped value
 * \return bool - false if log buffer is empty or invalid inputs.
 */
bool LogBufferCharPop( LogBuffer* pLb, int8_t* pChar );

/**
 * LogBufferClear - Resets the Log buffer. Data is not invalidated but all pointers
 * are reset and the data will be overwritten. 
 * 
 * \param pLb - pointer to valid Log Buffer
 */
void LogBufferClear( LogBuffer* pLb );
#endif

