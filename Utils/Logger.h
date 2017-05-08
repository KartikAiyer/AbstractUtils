/**
* The MIT License (MIT)
*
* Copyright (c) <2017> <Kartik Aiyer>
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

#ifndef __AUTILS_LOGGER_H__
#define __AUTILS_LOGGER_H__

#include <PlatformInterface.h>

typedef struct _SystemLogger
{
  size_t( *fnPrefix )( struct _SystemLogger*, void* pPrivate, char* pString, uint32_t stringSize );
  bool isEnabled;
}Logger;

typedef struct _SimpleSystemLogger
{
  Logger base;
  char* pPrefix;
}SimpleLogger;

size_t SimpleLoggerPrefix( Logger* pLogger,
                           void* pPrivate,
                           char* pString,
                           uint32_t stringSize );


#define SIMPLE_LOGGER_DECL( name ) \
SimpleLogger sysLog_##name

#define SIMPLE_LOGGER_DEF( name, prefix, enabled )\
SimpleLogger sysLog_##name = { .base.isEnabled = enabled,\
  .base.fnPrefix = SimpleLoggerPrefix,\
  .pPrefix = prefix\
}

#define SIMPLE_LOGGER_INIT( logger, prefix, enabled )\
(logger)->base.isEnabled = (enabled);\
(logger)->base.fnPrefix = SimpleLoggerPrefix;\
(logger)->pPrefix = (prefix)\

#define SIMPLE_LOGGER_STRUCT_INIT( name, prefix, enabled )\
sysLog_##name = {\
  .base.isEnabled = (enabled),\
  .base.fnPrefix = SimpleLoggerPrefix,\
  .pPrefix = prefix\
}

#define SIMPLE_LOGGER( name ) sysLog_##name

extern SIMPLE_LOGGER_DECL( GeneralLogger );

#endif // __AUTILS_LOGGER_H__