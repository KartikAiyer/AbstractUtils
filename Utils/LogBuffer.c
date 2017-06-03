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

#include <LogBuffer.h>

static void LogBufferCharPush( LogBuffer* pLb, char item );
bool LogBufferInit( LogBuffer* pLb, 
                    char* pBuffer, 
                    uint32_t bufferSize )
{
  bool retval = false;
  if( pLb && pBuffer && bufferSize ) {
    pLb->pBuffer = pBuffer;
    pLb->bufferSize = bufferSize;
    pLb->head = pLb->tail = 0;
    pLb->isInit = true;
    retval = true;
  }
  return retval;
}

/**
 * LogBufferFilterCharacters - Filter ansi escape sequences and
 * carriage return.
 *
 * return bool - false if no filtering needed, true if should filter
 *
 */
static bool LogBufferFilterCharacter( LogBuffer* pLb,
                                      char character )
{
  bool retval = true;

  // filter ansi escape sequences by looking for escape
  // start (0x1b) and escape end (letter character)
  if ( 0x1b == character ) {
    pLb->inEscapeSequence = true;
  } else {
    if ( pLb->inEscapeSequence ) {
      if ( ( character >= 'a' && character <= 'z' ) ||
           ( character >= 'A' && character <= 'Z') ) {
        pLb->inEscapeSequence = false;
      }
    } else {
      // filter out carriage return
      if ( 0xd != character ) {
        retval = false;
      }
    }
  }

  return retval;
}

bool LogBufferPush( LogBuffer* pLb, char* pString, uint32_t stringSize )
{
  bool retval = false;
  if( pLb && pLb->isInit && pString && stringSize < pLb->bufferSize ) {
    for( uint32_t i = 0; i < stringSize; i++ ) {
      char character = pString[i];
      LogBufferCharPush( pLb, character );
    }
    retval = true;
  }
  return retval;
}

bool LogBufferCharPop( LogBuffer* pLb, int8_t* pChar ) 
{
  bool retval = false;
  if ( pLb && pChar && 
       pLb->head != pLb->tail ) {
    *pChar = pLb->pBuffer[ pLb->tail ];
    pLb->tail = ( ( pLb->tail + 1 ) < pLb->bufferSize ) ? ( pLb->tail + 1 ) : 0;
    retval = true;
  }
  return retval;
}

bool LogBufferIsEmpty( LogBuffer* pLb )
{
  bool retval = false;
  if( pLb && pLb->isInit ) {
    retval = ( pLb->head == pLb->tail );
  }
  return retval;
}

static void LogBufferCharPush( LogBuffer* pLb, char item )
{
  if( pLb && item ) {
    pLb->pBuffer[ pLb->head ] = item;
    pLb->head = ( ( pLb->head + 1 ) < pLb->bufferSize ) ? ( pLb->head + 1 ) : 0;
    if( pLb->head == pLb->tail ) {
      //The Ring buffer was full... move the tail
      pLb->tail = ( ( pLb->tail +1 ) < pLb->bufferSize ) ? ( pLb->tail + 1 ) : 0;
    }
  }
}

void LogBufferClear( LogBuffer* pLb )
{
  if ( pLb && pLb->isInit ) {
    pLb->head = pLb->tail = 0;
  }
}
