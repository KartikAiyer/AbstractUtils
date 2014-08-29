/**
 * Confidential!!! 
 * Source code property of Blue Clover Design LLC.
 *  
 * Demonstration, distribution, replication, or other use of the 
 * source codes is NOT permitted without prior written consent 
 * from Blue Clover Design. 
 */

#include "Logable.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

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


