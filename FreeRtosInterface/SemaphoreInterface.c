
#include <SemaphoreInterface.h>

bool KSemaCreate( KSema* pSema, const char* pSemaName, uint32_t initalVal )
{
  bool retval = false;
  if( pSema ) {
    pSema->hSem = xSemaphoreCreateCountingStatic( 10, initalVal, &pSema->sem );
    retval = ( pSema->hSem ) ? true : false;
  }
  return retval;
}
void KSemaDelete( KSema* pSema )
{
  if( pSema ) {
    vSemaphoreDelete( pSema->hSem );
  }
}
bool KSemaGet( KSema* pSema, uint32_t timeout )
{
  bool retval = false;
  if( pSema ) {
    uint32_t frtosTimeout = TimeoutInMsToTickConverter( timeout );
    retval = ( xSemaphoreTake( pSema->hSem, frtosTimeout ) == pdTRUE ) ? true : false;
  }
  return retval;
}

void KSemaPut( KSema* pSema )
{
  if( pSema ) {
    xSemaphoreGive( pSema->hSem );
  }
}

