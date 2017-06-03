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

#include <ConsoleLog.h>
#include <Log.h>
#include <MutexInterface.h>

typedef struct _ConsoleLogStore
{
  TsLogBuffer *pCurrentLb;
  KMutex mtx;
  Logger generalLogger;
  bool isInit;
}ConsoleLogStore;

static ConsoleLogStore s_console;

void ConsoleLogInitialize()
{
  if ( !s_console.isInit ) {
    s_console.isInit = true;
    LogSystemInitialize();
    KMutexCreate( &s_console.mtx, "ConsoleLog" );
    s_console.generalLogger.fnPrefix = 0;
    s_console.generalLogger.isEnabled = true;
    s_console.pCurrentLb = LogSystemAllocateLogBuffer();
  }
}

void ConsoleLogFlush()
{
  if ( s_console.isInit ) {
    KMutexLock( &s_console.mtx, WAIT_FOREVER );
    TsLogBuffer* pLbToFlush = s_console.pCurrentLb;
    s_console.pCurrentLb = LogSystemAllocateLogBuffer();
    KMutexUnlock( &s_console.mtx );
    if ( pLbToFlush ) {
      LogSystemFlushLogBuffer( pLbToFlush );
    }
  }
}
void ConsoleLog( const char* pStr, ... )
{
  if ( s_console.isInit ) {
    va_list args;
    va_start( args, pStr );
    Log( s_console.pCurrentLb, &s_console.generalLogger, NULL, pStr, args );
    va_end( args );
  }
}

void ConsoleLogLine( const char* pStr, ... )
{ 
  if ( s_console.isInit ) {
    va_list args;
    va_start( args, pStr );
    LogLine( s_console.pCurrentLb, &s_console.generalLogger, NULL, pStr, args );
    va_end( args );
  }
}
