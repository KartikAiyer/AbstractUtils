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

#include <ThreadInterface.h>
#include <Logable.h>
#include <pthread.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

const uint32_t KThreadHandleSize = sizeof( KThread );

static void* Thread( void* arg )
{
  KThread* pThread = ( KThread* )arg;
#ifdef LINUX_PTHREAD
  pthread_setname_np( pThread->pthread, pThread->threadName );
#else
  pthread_setname_np( pThread->threadName );
#endif
  pThread->fn( pThread->arg );
  pThread->isComplete = true;
  return NULL;
}

bool KThreadCreate( KThread* pThread, const KThreadCreateParams* pParams )
{
  bool retval = false;
  int err = 0;
  if ( pThread && pParams && pParams->fn ) {
    pthread_attr_t attr;
    err = pthread_attr_init( &attr );
    if ( !err ) {
      struct sched_param schedParam = { 0 };
      err = pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
      if ( !err ) {
        err = pthread_attr_setstacksize( &attr, pParams->stackSizeInBytes );
        if( !err ) {
          schedParam.sched_priority = pParams->threadPriority;
          err = pthread_attr_setschedpolicy( &attr, SCHED_RR );
          if ( !err ) {
            pthread_attr_setschedparam( &attr, &schedParam );
            pThread->fn = pParams->fn;
            pThread->arg = pParams->threadArg;
            pThread->isComplete = false;
            pThread->sanity = THREAD_SANITY_CHECK;
            if( pParams->pThreadName ) {
              strncpy( pThread->threadName, pParams->pThreadName, sizeof( pThread->threadName ) );
            } else {
              memset( pThread->threadName, 0, sizeof( pThread->threadName ) );
            }
            err = pthread_create( &pThread->pthread, &attr, Thread, pThread );
            if ( !err ) {
              retval  = true;
            } else {
              LOG( "%s(): Unable to create thread. Err: %d", __FUNCTION__, err );
            }
          } else {
            LOG( "%s(): Unable to set Pthread Scheduling to OTHER: %d", __FUNCTION__, err );
          }
        } else {
          LOG( "%s(): Unable to set stack size to: %d, err: %d",
               __FUNCTION__, pParams->stackSizeInBytes, err );
        }
      } else {
        LOG( "%s(): Unable to make thread joinable. Err: %d", __FUNCTION__, err );
      }
      pthread_attr_destroy( &attr );
    }
    else {
      LOG( "%s(): Unable to initialize thread attribute data structure. Err: %d", __FUNCTION__, err );
    }
  } else {
    LOG( "%s(): Invalid Params", __FUNCTION__ );
  }
  return retval;
}

//TODO: Not sure if this implementation is correct. What does is mean to delete the Thread
//Does Joining the thread make sure that the thread is deleted? 
bool KThreadDelete( KThread* pThread )
{
  return KThreadJoin( pThread );
}

bool KThreadJoin( KThread* pThread )
{
  bool retval = true;
  int err = 0;
  if ( pThread && !pThread->isComplete ) {
    err = pthread_join( pThread->pthread, NULL );
    if ( err ) {
      LOG( "%s(): Was not able to wait for thread to join. Err: %d", __FUNCTION__, err );
      retval = false;
    }
  } 
  return retval;
}

int32_t KThreadGetPriority( KThread* pThread )
{
  struct sched_param params = { 0 };
  int policy;

  pthread_getschedparam( pThread->pthread, &policy, &params );
  return params.sched_priority;
}

const char* KThreadGetName( KThread* pThread )
{
  return pThread->threadName;
}

#ifdef __cplusplus
}
#endif
