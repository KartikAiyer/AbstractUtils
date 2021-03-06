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
#include <MutexInterface.h>

bool  KMutexCreate( KMutex* pMutex, const char* pMutexName )
{
  bool retval = false;
  if( pMutex ) {
    pMutex->hSem = xSemaphoreCreateMutexStatic( &pMutex->sem );
    if( pMutexName ){
      strncpy_s( pMutex->mutexName, sizeof( pMutex->mutexName ), pMutexName, sizeof( pMutex->mutexName ) - 1  );
    }
    retval = ( pMutex->hSem ) ? true : false;
  }
  return retval;
}
void KMutexDelete( KMutex* pMutex )
{
  if( pMutex ) {
    vSemaphoreDelete( pMutex->hSem );
  }
}
bool KMutexLock( KMutex* pMutex, uint32_t timeout )
{
  bool retval = false;
  if( pMutex ) {
    uint32_t frtosTimeout = TimeoutInMsToTickConverter( timeout );
    retval = ( xSemaphoreTake( pMutex->hSem, frtosTimeout ) == pdTRUE ) ? true : false;
  }
  return retval;
}
void KMutexUnlock( KMutex* pMutex )
{
  if( pMutex ){
    xSemaphoreGive( pMutex->hSem );
  }
}