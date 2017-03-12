#include <MutexInterface.h>

bool  KMutexCreate( KMutex* pMutex, const char* pMutexName )
{
  bool retval = false;
  if( pMutex ) {
    pMutex->hSem = xSemaphoreCreateMutexStatic( &pMutex->sem );
    if( pMutexName ){
      strncpy( pMutex->mutexName, pMutexName, sizeof( pMutex->mutexName ) - 1  );
    }
    retval = ( pMutex->hSem ) ? true : false;
  }
  return retval;
}
void KMutexDelete( KMutex* pMutex )
{
  if( pMutex ) {
    vSemaphoreDelete( pMutex->hSem );
  }
}
bool KMutexLock( KMutex* pMutex, uint32_t timeout )
{
  bool retval = false;
  if( pMutex ) {
    uint32_t frtosTimeout = TimeoutInMsToTickConverter( timeout );
    retval = ( xSemaphoreTake( pMutex->hSem, frtosTimeout ) == pdTRUE ) ? true : false;
  }
  return retval;
}
void KMutexUnlock( KMutex* pMutex )
{
  if( pMutex ){
    xSemaphoreGive( pMutex->hSem );
  }
}