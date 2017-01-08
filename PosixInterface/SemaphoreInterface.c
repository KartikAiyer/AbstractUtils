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

#include <SemaphoreInterface.h>
#include <semaphore.h>
#ifdef LINUX_PTHREAD // On MAC OSX I don't need these
#include <pthread.h>
#include <fcntl.h>
#include <limits.h>
#endif
#include <Logable.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

bool KSemaCreate( KSema* pSema, const char* pSemaName, uint32_t initialVal )
{
  bool retval = false;
  int status = 0;
  if ( pSema && initialVal < SEM_VALUE_MAX ) {
    int16_t initVal16 = initialVal;
    size_t strLen = strlen( pSemaName );
    pSema->pSemaphoreName = malloc( strLen + 1 );
    if( pSema->pSemaphoreName ) {
      strcpy( pSema->pSemaphoreName, pSemaName );
    }
    sem_unlink( pSemaName );
    pSema->pNamedSema = sem_open( pSema->pSemaphoreName, O_CREAT, S_IRWXU, initVal16 );
    if ( pSema->pNamedSema != SEM_FAILED ) {
      retval = true;
    }
    else {
      LOG( "%s(): Couldn't create Semaphore. Return Status: %d", __FUNCTION__, status );
    }
  }
  return retval;
}

void KSemaDelete( KSema* pSema )
{
  if ( pSema ) {
    int status = sem_close( pSema->pNamedSema );
    if ( !status ) {
      status = sem_unlink( pSema->pSemaphoreName );
      free( pSema->pSemaphoreName );
      pSema->pSemaphoreName = 0;
    } else {
      LOG( "%s(): Unable to destroy semaphore. RetStatus: %d", __FUNCTION__, status );
    }
  }
}

bool KSemaGet( KSema* pSema, uint32_t timeout )
{
  bool retval = false;
  int status = 0;
  if ( pSema ) {
    if ( timeout == WAIT_FOREVER ) {
      status = sem_wait( pSema->pNamedSema );
      if ( status ) {
        LOG( "%s(): Error while waiting for semaphore (%d)", __FUNCTION__, status );
      } else {
        retval = true;
      }
    } else if( timeout == NO_SLEEP ) {
      int val = 0;
      status = sem_trywait( pSema->pNamedSema );
      if ( !status ) {
        retval = true;
      } else {
        LOG( "%s(): Error while trying to peek at sema value (%d)", __FUNCTION__, status );
      }
    } else {
      LOG( "%s(): Timed waiting on semaphore not implemented for pthreads", __FUNCTION__ ); 
    }
  }
  return retval;
}

void KSemaPut( KSema* pSema )
{
  if ( pSema ) {
    int status = 0;
    status = sem_post( pSema->pNamedSema );
    if ( status ) {
      LOG( "%s(): Unable to post semaphore (%d)", __FUNCTION__, status );
    }
  }
}

#ifdef __cplusplus
}
#endif
