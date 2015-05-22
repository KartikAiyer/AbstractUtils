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
#include <pthread.h>
#include <Logable.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

bool  KMutexCreate( KMutex* pMutex, const char* pMutexName )
{
  bool retval = false;
  pthread_mutexattr_t attr;
  if ( pMutex ) {
    pthread_mutexattr_init( &attr );
    if ( pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK ) == 0 ) {
      if ( pthread_mutexattr_setprotocol( &attr, PTHREAD_PRIO_INHERIT ) == 0 ) {
        if ( pthread_mutex_init( pMutex, &attr ) == 0 ) {
          retval = true;
        }
        else{
          LOG( "%s(): Couldn't Initialize Mutex %s", __FUNCTION__, pMutexName );
        }
      }
      else
      {
        LOG( "%s(): Couldn't set Mutex Protocol", __FUNCTION__ );
      }
    } else {
      LOG( "%s(): Couldn't set Mutex Type", __FUNCTION__ );
    }
    pthread_mutexattr_destroy( &attr );
  }
  return retval;
}

void KMutexDelete( KMutex* pMutex )
{
  if ( pMutex ) {
    pthread_mutex_destroy( pMutex );
  }
}

bool KMutexLock( KMutex* pMutex, uint32_t timeout )
{
  bool retval = false;
  if ( pMutex ) {
    if ( timeout == NO_SLEEP || timeout != WAIT_FOREVER ) {
      if ( timeout == NO_SLEEP && 
           pthread_mutex_trylock( pMutex ) == 0 ) {
        //Acquired Lock
        retval = true;
      } else if( pthread_mutex_lock( pMutex ) == 0 ) {
        retval = true;
      }
    }
  }
  assert( retval == true );
  return retval;
}

void KMutexUnlock( KMutex* pMutex )
{
  if( pMutex ) {
    pthread_mutex_unlock( pMutex );
  }
}

#ifdef __cplusplus
}
#endif
