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

#include <Logable.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

Logable g_generalLogger = { .prefix = 0, .enabled = true };  

void Log( Logable *pLog, const char* pStr, ... )
{
  va_list arguments;
  va_start(arguments, pStr );
  if( pLog && pLog->enabled ) {
    uint32_t time = 0;
    if( pLog->prefix ) {
      printf("[%010u ms - %s]: ", ( unsigned int )time, pLog->prefix );
    }
    else{
      printf( "[%010u ms]: ", ( unsigned int )time );
    }
    vprintf( pStr, arguments );
    printf("\n");
  }
  va_end( arguments );
}

#ifdef __cplusplus
}
#endif

