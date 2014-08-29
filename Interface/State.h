/**
 * Confidential!!! 
 * Source code property of Blue Clover Design LLC.
 *  
 * Demonstration, distribution, replication, or other use of the 
 * source codes is NOT permitted without prior written consent 
 * from Blue Clover Design. 
 */
#ifndef __STATE_H__
#define __STATE_H__

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "Utilities/Logable.h"
#include "Utilities/klist.h"

///Forward Declarations
struct _State;

typedef enum {
  STATE_OK   = 0,
  STATE_ERROR = 1,
}StateStatus;

///Callback Prototypes

/**
 * The StateInit callback function is called after the specific state has been created. 
 *  
 * The function assumes that the State passed is allocated of 
 * the appropriate type. ie. If you call funtion pointer on say 
 * StateReady type object, you need to pass the same StateReady 
 * object in as a parameter. 
 *  
 *  
 * \param pState State* An allocated and initialized state 
 *                      sruct.
 */
typedef void (*StateInit)( struct _State* pState );

/**
 * Function pointer is called when entering the state. 
 *  
 * \param pState State* A valid state pointer 
 */
typedef void (*StateEnter)( struct _State* pState );

/**
 * Funtion pointer is called when exiting the state
 * 
 * \param pState State* A valid State pointer
 */
typedef void (*StateExit)( struct _State* pState );
/**
 * Query function which responds true if the event can be 
 * handled by the state. 
 *  
 * \param pState State* A Valid state pointer 
 * \param pEvetn AppEvent* pointer to event received. 
 *  
 * \return bool True if event can be handled false otherwise.
 */
typedef bool (*StateIsValidEvent)( struct _State *pState, void *pEvent );

/**
 * Function pointer to State Event Handler. 
 *  
 * The function handles the incoming event and returns state ID 
 * to transition to. The returned state Id can be the current 
 * state indicating that no state transition is required. 
 *  
 * \param pState State* Pointer to current State 
 * \param pEvent AppEvent* Pointer to received event. 
 *  
 * \return uint32_t A desitination stateId. This can be the 
 *         current stateId as well thus implying no state
 *         transition required.
 */
typedef uint32_t (*StateHandleEvent)( struct _State *pState, void *pEvent );


#define STATE_LOG(pState, str, ...)          (Log( (&(pState->dbg.log)), str, ##__VA_ARGS__))
#define STATE_DEFAULT_LOGGING                (true)
typedef struct _StateDebug
{
  uint32_t numOfEvents;
  Logable log;
}StateDebug;

/**
 * \struct State 
 * \brief Data structure that encapsulates an application state. 
 *  
 * The data structure is the base representation of a state 
 * which holds all the generic state handling function pointers. 
 * Specific states should have an instance of this data 
 * structure as the first memeber of their data structure. 
 * eg.
  
 * typedef struct _StateReady 
 * {
 *   State base;
 *   bool isReady;
 * }StateReady; 
 *  
 * The data structure can be added to a Klist. Refer to klist.h 
 */
typedef struct _State
{
  /** List elem so that it can be queued in a list. Must be
   *  first element */
  KListElem listElem;
  /** a unique stateId across states to be used in a state
   *  machine */
  uint32_t stateId;
  /** For Debug purposes */
  StateDebug dbg;
  /** Static string as state name */
  const char *pStateName;
  /** Pointer to init function (Optional). This function is called
   *  once when the State machine is started. This function is
   *  optiona Pointer to init function. This function is called
   *  once when the State machine is started. This function is
   *  optional
   */
  StateInit fnInit;
  /** Pointer to function to be called when state is entered. */
  StateEnter fnEnter;
  /** Poiunter to function when state is exited. */
  StateExit  fnExit;
  /** Pointer to query function that checks if a state can
   *  handle a specific event. */
  StateIsValidEvent fnValidEvent;
  /** Pointer to State specific event handler. */
  StateHandleEvent fnHandleEvent;
}State;

/** 
 *  Macro to initalize a state data structure.
 *  
 *  \param stId: uint32_t - a unique state Id
 *  \param stateName: const char* - state name string
 *  \param init: StateInit - init function pointer
 *  \param Enter: StateEnter - entry function pointer
 *  \param Exit: StateExit - exit function pointer
 *  \param validEvent: StateIsValidEvent - valid event query
 *  \param handleEvent: StateHandleEvent - event handler
 *  \param logging: bool - Logging Enabled/Disabled
 */
#define STATE_INITIALIZER( stId, stateName, init, Enter, Exit, validEvent, handleEvent, logging ) \
{\
  .listElem = { 0 },\
  .stateId = stId,\
  .pStateName = stateName,\
  .dbg = { .log = { .prefix = stateName, .enabled = logging }, .numOfEvents = 0 },\
  .fnInit = init,\
  .fnEnter = Enter,\
  .fnExit = Exit,\
  .fnValidEvent = validEvent,\
  .fnHandleEvent = handleEvent\
}

#endif // __STATE_H__

