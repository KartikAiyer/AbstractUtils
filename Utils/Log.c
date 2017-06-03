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

#include <AbstractUtilsConfig.h>
#include <Log.h>
#include <Pool.h>
#include <MutexInterface.h>
#include <stdio.h>


#ifdef CONFIG_USE_AUTILS_LOG_SYSTEM	
#define LOG_POOL_SIZE         POOL_STORE_SIZE( LOG_POOL_MAX_LOG_BUFFERS, sizeof( TsLogBuffer ) + LOG_POOL_LOG_BUFFER_SIZE )

typedef struct _LogPool
{
  uint8_t logBufferStore[ LOG_POOL_SIZE ];
  MemPool logBufferPool;
  bool isInit;
  KMutex mtx;
}LogPool;

static LogPool s_logPool = { 0 };

void LogSystemInitialize()
{
  if ( !s_logPool.isInit ) {
    if ( KMutexCreate( &s_logPool.mtx, "LogPoolMutex" ) ) {
      PoolCreate( &s_logPool.logBufferPool, s_logPool.logBufferStore, LOG_POOL_SIZE, LOG_POOL_MAX_LOG_BUFFERS );
      s_logPool.isInit = true;
    }
  }
}

TsLogBuffer* LogSystemAllocateLogBuffer()
{
  TsLogBuffer* retval = 0;
  uint32_t tsLogBufSize = sizeof( TsLogBuffer );
  if ( s_logPool.isInit ) {
    retval = (TsLogBuffer*) PoolAlloc( &s_logPool.logBufferPool );
    if ( retval ) {
      memset( retval, 0, tsLogBufSize );
      if ( !TsLogBufferInit( &retval->lb, (uint8_t*)(retval) + tsLogBufSize, LOG_POOL_LOG_BUFFER_SIZE, "LogBuffer" ) )
        PoolFree( &s_logPool.logBufferPool, retval );
        retval = 0;
      }
    }
  }
  return retval;
}

void LogSystemFreeLogBuffer( TsLogBuffer* pLogBuffer )
{
  if ( s_logPool.isInit && pLogBuffer ) {
    KMutexDelete( &pLogBuffer->mtx );
    PoolFree( &s_logPool.logBufferPool, pLogBuffer );
  }
}

bool LogSystemFlushLogBuffer( TsLogBuffer* pLb )
{
#error "NEED IMPLEMENTATION"
  return false;
}
#endif


bool TsLogBufferInit( TsLogBuffer* pLb, char* pBuffer, uint32_t bufferSize, char* logBufferName )
{
  bool retval = false;
  if ( pLb ) {
    if ( KMutexCreate( &pLb->mtx, logBufferName ) ) {
      retval = LogBufferInit( &pLb->lb, pBuffer, bufferSize );
    }
  }
  return retval;
}

static int32_t LogWriteString( Logger* pLogger, void* pPrivate, const char* pStr, char* pBuffer, va_list args )
{
  size_t strLen = strlen( pStr );
  int32_t index = 0;
  if ( strLen < ( LOG_MAX_LINE_LENGTH - 1 ) / 2 ) {
    if ( pLogger->fnPrefix ) {
      index += pLogger->fnPrefix( pLogger,
                                  pPrivate,
                                  pBuffer + index,
                                  LOG_MAX_LINE_LENGTH - index - strLen );
      pBuffer[ index++ ] = ':';
      pBuffer[ index++ ] = ' ';
    }
    index += vsprintf_s( pBuffer + index, LOG_MAX_LINE_LENGTH - 1 - index, pStr, args );
  }
  return index;
}

void Log( TsLogBuffer* pLb, Logger* pLogger, void* pPrivate, const char* fmt, va_list args )
{
  if ( pLb && pLb->lb.isInit && pLogger && pLogger->isEnabled ) {
    char buffer[ LOG_MAX_LINE_LENGTH ];
    int32_t index = LogWriteString( pLogger, pPrivate, fmt, buffer, args );
    if ( index ) {
      KMutexLock( &pLb->mtx, WAIT_FOREVER );
      LogBufferPush( &pLb->lb, buffer, index );
      KMutexUnlock( &pLb->mtx );
    }
    va_end( args );
  }
}

void LogLine( TsLogBuffer* pLb, Logger* pLogger, void* pPrivate, const char* fmt, va_list args )
{
  if ( pLb && pLb->lb.isInit && pLogger && pLogger->isEnabled ) {
    char buffer[ LOG_MAX_LINE_LENGTH ];
    int32_t index = LogWriteString( pLogger, pPrivate, fmt, buffer, args );
    if ( index ) {
      index += sprintf_s( buffer + index, LOG_MAX_LINE_LENGTH - 1 - index, "\n" );

      KMutexLock( &pLb->mtx, WAIT_FOREVER );
      LogBufferPush( &pLb->lb, buffer, index );
      KMutexUnlock( &pLb->mtx );
    }
  }
}