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

#include <PlatformInterface.h>
#include <klist.h>
#include <windows.h>
#include <stdio.h>
#include <Log.h>


typedef struct _LogBufferStoreT
{
  SLIST_ENTRY elem;
  TsLogBuffer lb;
  uint8_t backingStore[ WINPORT_CONSOLE_LOG_BUFFER_SIZE ];
}LogBufferStoreT;
/**
 * We need a separate windows thread that is associated with all cores that are not set to the freeRTOS core
 * to use for offloading windows system calls in order to avoid context switching of the FreeRTOS threads
 * outside of the FreeRTOS thread scheduler.
 */

typedef struct _WindowsPortData
{
  HANDLE hSyslogThread;
  HANDLE hLoggingThreadEvent;
  FILE* hFile;
  bool stdoutLogging;
  SLIST_HEADER* pHeadLogFreeList;
  SLIST_HEADER* pHeadLogFlushList;
  bool isInit;
  bool killRequest;
}WindowsPortData;

static WindowsPortData s_winData;

/* This thread will consume all LogBuffer producers
 * Thus All LogBuffer producers must be given their TsLogBuffers by this module
 * Currently the only LogBuffer producer is ConsoleLog.
 */

bool WinPortInit();
void WinPortDeInit();

static DWORD WINAPI Win32LoggingThread( void *pvParameter )
{
  DWORD retval = 0;
  for ( ;!s_winData.killRequest; ) {
    retval = WaitForSingleObject( s_winData.hLoggingThreadEvent, 1000 );
    if ( retval == WAIT_TIMEOUT && !s_winData.killRequest ) {
      continue;
    } else if ( retval == WAIT_OBJECT_0 && !s_winData.killRequest ) {
      LogBufferStoreT* pItem = ( LogBufferStoreT* ) InterlockedPopEntrySList( s_winData.pHeadLogFlushList );
      if ( pItem ) {
        int8_t c = 0;
        while ( LogBufferCharPop( &pItem->lb.lb, &c ) ){
          putc( c, stdout );
        }
        LogBufferClear( &pItem->lb.lb );
        InterlockedPushEntrySList( s_winData.pHeadLogFreeList, &pItem->elem );
      }
    } else if( !s_winData.killRequest ) {
      //Error
      retval = -1;
      break;
    }
  }
  return retval;
}

static void ClearList( SLIST_HEADER* pList )
{
  LogBufferStoreT* pItem = NULL;
  if ( pList ) {
    do {
      pItem = ( LogBufferStoreT* )InterlockedPopEntrySList( pList );
      if ( pItem ) {
        _aligned_free( pItem );
      } else {
        break;
      }
    } while ( pItem );
  }
}

static void DeleteList( SLIST_HEADER* pList )
{
  if ( pList ) {
    ClearList( pList );
    _aligned_free( pList );
  }
}

void LogSystemInitialize()
{
  WinPortInit();
}

TsLogBuffer* LogSystemAllocateLogBuffer()
{
  TsLogBuffer* retval = 0;
  if ( s_winData.isInit ) {
    LogBufferStoreT* pItem = ( LogBufferStoreT* )InterlockedPopEntrySList( s_winData.pHeadLogFreeList );
    retval = &pItem->lb;
  }
  return retval;
}

void LogSystemFreeLogBuffer( TsLogBuffer* pLb )
{
  if ( pLb ) {
    LogBufferStoreT* pItem = (LogBufferStoreT*) ( (uint8_t*)pLb - offsetof( LogBufferStoreT, lb ) );
    InterlockedPushEntrySList( s_winData.pHeadLogFreeList, &pItem->elem );
  }
}

bool LogSystemFlushLogBuffer( TsLogBuffer* pLb )
{
  bool retval = false;
  if ( s_winData.isInit && 
       pLb && pLb->lb.isInit ) {
    LogBufferStoreT* pItem = (LogBufferStoreT*) ( (uint8_t*)pLb - offsetof( LogBufferStoreT, lb ) );
    InterlockedPushEntrySList( s_winData.pHeadLogFlushList, &pItem->elem );
    SetEvent( s_winData.hLoggingThreadEvent );
    retval = true;
  }
  return retval;
}

bool WinPortInit()
{
  bool retval = true;
  if ( !s_winData.isInit ) {
    /* Create the buffer. */
    s_winData.pHeadLogFreeList = ( SLIST_HEADER* )_aligned_malloc( sizeof( SLIST_HEADER ), MEMORY_ALLOCATION_ALIGNMENT );
    s_winData.pHeadLogFlushList = ( SLIST_HEADER* )_aligned_malloc( sizeof( SLIST_HEADER ), MEMORY_ALLOCATION_ALIGNMENT );
    if ( s_winData.pHeadLogFreeList && s_winData.pHeadLogFlushList ) {
      InitializeSListHead( s_winData.pHeadLogFlushList );
      InitializeSListHead( s_winData.pHeadLogFreeList );
      for ( uint8_t i = 0; retval && i < WINPORT_CONSOLE_LOG_NUM_SWAP_BUFS; i++ ) {
        LogBufferStoreT* pLbStore = ( LogBufferStoreT* )_aligned_malloc( sizeof( LogBufferStoreT ), MEMORY_ALLOCATION_ALIGNMENT );
        if ( !pLbStore ) {
          DeleteList( s_winData.pHeadLogFreeList );
          DeleteList( s_winData.pHeadLogFlushList );
          s_winData.pHeadLogFreeList = NULL;
          retval = false;
          continue;
        }
        TsLogBufferInit( &pLbStore->lb, pLbStore->backingStore, sizeof( pLbStore->backingStore ), "ConsoleLog" );
        InterlockedPushEntrySList( s_winData.pHeadLogFreeList, &pLbStore->elem );
      }
    }
    if ( retval ) {
      /* Create the Windows event. */
      s_winData.hLoggingThreadEvent = CreateEvent( NULL, FALSE, TRUE, "StdoutLoggingEvent" );
      if ( s_winData.hLoggingThreadEvent ) {
        /* Create the thread itself. */
        s_winData.hSyslogThread = CreateThread(
          NULL,	/* Pointer to thread security attributes. */
          0,		/* Initial thread stack size, in bytes. */
          Win32LoggingThread,	/* Pointer to thread function. */
          NULL,	/* Argument for new thread. */
          0,		/* Creation flags. */
          NULL );

        /* Use the cores that are not used by the FreeRTOS tasks. */
        SetThreadAffinityMask( s_winData.hSyslogThread, ~0x01u );
        SetThreadPriorityBoost( s_winData.hSyslogThread, TRUE );
        SetThreadPriority( s_winData.hSyslogThread, THREAD_PRIORITY_IDLE );
        s_winData.isInit = true;
      } else {
        retval = false;
      }
    }
  }
  return retval;
}

void WinPortDeInit()
{
  if ( s_winData.isInit ) {
    s_winData.isInit = false;
    s_winData.killRequest = true;
    SetEvent( s_winData.hLoggingThreadEvent );
    if ( WaitForSingleObject( s_winData.hSyslogThread, 1000 ) != WAIT_OBJECT_0 ) {
      TerminateThread( s_winData.hSyslogThread, 2 );
    }
    DeleteList( s_winData.pHeadLogFlushList );
    DeleteList( s_winData.pHeadLogFreeList );
  }
}