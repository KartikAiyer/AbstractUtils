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
#include <MutexInterface.h>
#include <ConsoleLog.h>

#ifdef __cplusplus
extern "C" {
#endif

static void ThreadLauncher( void* arg )
{
  KThread* pThread = ( KThread* )arg;
  for( ;; ) {
    TaskHandle_t hJoin;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY );
    pThread->fn( pThread->arg );
    KMutexLock( &pThread->joinMutex, WAIT_FOREVER );
    hJoin = pThread->hJoinRequestTask;
    KMutexUnlock( &pThread->joinMutex );
    if( hJoin ) {
      xTaskNotifyGive( hJoin );
    }
    vTaskDelete(NULL);
  }
}

bool KThreadCreate( KThread* pThread, const KThreadCreateParams* pParams )
{
  bool retval = false;
  int err = 0;
  if ( pThread &&
      pParams &&
      pParams->fn &&
      pParams->pStack &&
      pParams->stackSizeInBytes / sizeof( StackType_t ) > configMINIMAL_STACK_SIZE  ) {
    StackType_t *pStack = ( StackType_t * ) pParams->pStack;
    memset( pThread, 0, sizeof( KThread ) );
    pThread->fn = pParams->fn;
    pThread->arg = pParams->threadArg;
    pThread->isComplete = false;
    pThread->sanity = THREAD_SANITY_CHECK;
    if ( pParams->pThreadName ) {
      strncpy_s( pThread->threadName, sizeof( pThread->threadName ), pParams->pThreadName, sizeof( pThread->threadName ));
    } else {
      memset( pThread->threadName, 0, sizeof( pThread->threadName ));
    }
    if ( KMutexCreate( &pThread->joinMutex, NULL ) ) {
      pThread->hTask = xTaskCreateStatic( ThreadLauncher,
                                          pThread->threadName,
                                          pParams->stackSizeInBytes / sizeof( StackType_t ),
                                          pThread,
                                          pParams->threadPriority,
                                          pStack,
                                          &pThread->task );
      if ( pThread->hTask ) {
        xTaskNotifyGive( pThread->hTask );
        retval = true;
      }
    }
  }
  return retval;
}

bool KThreadJoin( KThread* pThread )
{
  bool retval = false;
  if( xTaskGetCurrentTaskHandle() != pThread->hTask ) {
    if ( eTaskGetState( pThread->hTask ) != eDeleted ) {
      //Wait to be notified when deleted
      KMutexLock( &pThread->joinMutex, WAIT_FOREVER );
      if( !pThread->hJoinRequestTask ){
        pThread->hJoinRequestTask = xTaskGetCurrentTaskHandle();
        retval = true;
      }
      KMutexUnlock( &pThread->joinMutex );
      if( retval ) ulTaskNotifyTake(pdTRUE, portMAX_DELAY );
    } else {
      retval = true;
    }
  }
  return retval;
}

bool KThreadDelete( KThread* pThread )
{
  bool retval = false;
  if( pThread ) {
    KThreadJoin( pThread );
    pThread->hTask = 0;
    pThread->sanity = 0;

    KMutexDelete( &pThread->joinMutex );
    pThread->hJoinRequestTask = 0;
    retval = true;
  }
  return retval;
}

int32_t KThreadGetPriority( KThread* pThread )
{
  return (int32_t)uxTaskPriorityGet( pThread->hTask );
}

const char* KThreadGetName( KThread* pThread )
{
  return pThread->threadName;
}

void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{
  volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

  /* Called if an assertion passed to configASSERT() fails.  See
  http://www.freertos.org/a00110.html#configASSERT for more information. */

  /* Parameters are not used. */
  ( void )ulLine;
  ( void )pcFileName;

  ConsoleLogLine( "ASSERT! Line %d, file %s\r\n", ulLine, pcFileName );

  taskENTER_CRITICAL();
  {
    /* You can step out of this function to debug the assertion by using
    the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
    value. */
    while ( ulSetToNonZeroInDebuggerToContinue == 0 ) {
      __asm { NOP };
      __asm { NOP };
    }
  }
  taskEXIT_CRITICAL();
}

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  /* If the buffers to be provided to the Idle task are declared inside this
  function then they must be declared static - otherwise they will be allocated on
  the stack and so not exists after this function exits. */
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  /* If the buffers to be provided to the Timer task are declared inside this
  function then they must be declared static - otherwise they will be allocated on
  the stack and so not exists after this function exits. */
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

  /* Pass out a pointer to the StaticTask_t structure in which the Timer
  task's state will be stored. */
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

  /* Pass out the array that will be used as the Timer task's stack. */
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;

  /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void KThreadInit()
{
  /* Other init before calling the scheduler  */
  ConsoleLogInitialize();
  /* Start the scheduler so the demo tasks start to execute. */
  vTaskStartScheduler();

  /* vTaskStartScheduler() would only return if RAM required by the Idle and
  Timer tasks could not be allocated.  As this demo uses statically allocated
  RAM only, there are no allocations that could fail, and
  vTaskStartScheduler() cannot return - so there is no need to put the normal
  infinite loop after the call to vTaskStartScheduler(). */
}

#ifdef __cplusplus
}
#endif
