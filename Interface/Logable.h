/**
 * Confidential!!! 
 * Source code property of Blue Clover Design LLC.
 *  
 * Demonstration, distribution, replication, or other use of the 
 * source codes is NOT permitted without prior written consent 
 * from Blue Clover Design. 
 */
#ifndef __LOGGABLE_H__
#define __LOGGABLE_H__

typedef void (*Logger)( char* str, ... );
typedef struct _Logable
{
  unsigned char enabled;
  const char* prefix; 
}Logable;


void Log( Logable *pLoggable, const char* str, ... );

extern Logable g_generalLogger;  

#define LOG( str, ... )       Log( &g_generalLogger, str, ##__VA_ARGS__ )

#endif //__LOGGABLE_H__
