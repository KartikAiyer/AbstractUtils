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

#include "StateMachine.h"
#include <string.h>
#include <assert.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

#define SM_LOG(str, ...)          (ConsoleLogLine( str, ##__VA_ARGS__))
#define SM_DEFAULT_LOGGING          (true)

static void StateMachineInitialize( StateMachine* pSm )
{
  memset( pSm, 0, sizeof(StateMachine) );
}

StateMachine* StateMachineCreate( const StateMachineInitParams* pInit )
{
  assert( pInit->fnAlloc );
  StateMachine *pStateMac = pInit->fnAlloc();
  assert( pStateMac );
  StateMachineInitialize( pStateMac );
  pStateMac->fnMachineAlloc = pInit->fnAlloc;
  pStateMac->fnMachineDealloc = pInit->fnDealloc;
  pStateMac->fnMachineInit = pInit->fnInit;
  pStateMac->pPrivate = pInit->pPrivate;

  if ( pStateMac->fnMachineInit ) {
    pStateMac->fnMachineInit( pStateMac );
  }
  SM_LOG( "Created %s", pInit->pName );

  return pStateMac;
}

State* StateMachineGetState( StateMachine* pStateMac, uint32_t stateId )
{
  State* pRetval = NULL;
  uint32_t i = 0;
  for ( i = 0; i < pStateMac->numOfStates; i++ ) {
    if ( pStateMac->states[ i ]->stateId == stateId ) {
      pRetval = pStateMac->states[ i ];
      break;
    }
  }

  return pRetval;
}

void StateMachineStart( StateMachine* pStateMac, uint32_t initialState )
{
  State* pInitialState = StateMachineGetState( pStateMac, initialState );
  uint32_t i = 0;
  //Set up the first state to load the state machine
  assert( pInitialState );
  pStateMac->currentState = pInitialState;

  //Call the init functions of all the states.
  for ( i = 0; i < pStateMac->numOfStates; i++ ) {
    State* pState = pStateMac->states[ i ];
    if ( pState->fnInit ) {
      pState->fnInit( pState );
    }
  }
  pStateMac->stateMachineStarted = true;
  //Enter the initial state.
  pStateMac->currentState->fnEnter( pStateMac->currentState );
}

void StateMachineStop( StateMachine* pStateMac )
{
  if ( pStateMac ) {
    SM_LOG( "Stopping State Machine" );
    if ( pStateMac->currentState ) {
      pStateMac->currentState->fnExit( pStateMac->currentState );
      pStateMac->currentState = 0;
      pStateMac->transitionRequested = false;
    }
    SM_LOG( "State Machine Stopped");
  }
}

void StateMachineRegisterState( StateMachine* pStateMac, State* pState )
{
  if ( pStateMac && pState && ( pStateMac->numOfStates < STATE_MAC_MAX_STATES ) ) {
    //Confirm presence of necessary functions
    assert( pState->fnEnter );
    assert( pState->fnExit );
    assert( pState->fnValidEvent );
    assert( pState->fnHandleEvent );
    pStateMac->states[ pStateMac->numOfStates++ ] = pState;
  }
}

void StateMachineHandleEvent( StateMachine* pStateMac, void* pEvent )
{
  if ( pEvent && pStateMac->stateMachineStarted ) {
    if ( pStateMac->currentState->fnValidEvent( pStateMac->currentState, pEvent ) ) {
      pStateMac->nextStateRequested = pStateMac->currentState->fnHandleEvent( pStateMac->currentState, pEvent );

      //Will transition when event handling is completed and StateTransition() is invoked.
      if ( pStateMac->nextStateRequested != pStateMac->currentState->stateId ) {
        pStateMac->transitionRequested = true;
      }
    }
  }
}

void StateMachineTransition( StateMachine* pStateMac )
{
  if ( pStateMac->transitionRequested && pStateMac->stateMachineStarted ) {
    pStateMac->transitionRequested = false;

    if ( pStateMac->currentState->stateId != pStateMac->nextStateRequested ) {
      State* pNextState = StateMachineGetState( pStateMac, pStateMac->nextStateRequested );
      if ( !pNextState ) {
        SM_LOG( "Did not find State: %d. FATAL!!!", pStateMac->nextStateRequested );
        assert( pNextState );
      }
      SM_LOG( "Transition: %s -> %s", pStateMac->currentState->pStateName, pNextState->pStateName );
      pStateMac->currentState->fnExit( pStateMac->currentState );
      pStateMac->currentState = pNextState;
      pStateMac->currentState->fnEnter( pStateMac->currentState );
    }
  }
}

#ifdef __cplusplus
}
#endif
