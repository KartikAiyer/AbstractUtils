/**
 * Confidential!!! 
 * Source code property of Blue Clover Design LLC.
 *  
 * Demonstration, distribution, replication, or other use of the 
 * source codes is NOT permitted without prior written consent 
 * from Blue Clover Design. 
 */
#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__


#include "State.h"
/** Maximum number of states that a state machine can have. */
#define STATE_MAC_MAX_STATES        20

struct _StateMachine;

typedef struct _StateMachine* (*StateMachineAllocate)( void );
typedef void (*StateMachineInit)( struct _StateMachine* pMac );
typedef void (*StateMachineDealloc)( struct _StateMachine** ppMac );

typedef struct _StateMachineInitParams
{
  const char *pName;
  void* pPrivate;
  StateMachineAllocate fnAlloc;
  StateMachineInit fnInit;
  StateMachineDealloc fnDealloc;
}StateMachineInitParams;

/**
 * \struct StateMachine 
 * \brief Encapsulates a state machine 
 *  
 * This data structure encapsulates data specific to a state 
 * machine. A state machine contains an array of state pointers, 
 * which are registered with the state machine during the state 
 * machine's configuration. The configuration steps include 
 * 1) Create the state machine 
 * 2) Configure the State machine with all the states 
 * 3) Start the state machine. 
 *  
 */
typedef struct _StateMachine
{
  /** Number of states in the state machine. Used when
   *  iterating through state array */
  uint32_t numOfStates;
  /** Pointer to the current state. This is passed to the
   *  appropriate state functions and event handlers. */
  State*   currentState;
  /** Array of all states in the state machine. */
  State*   states[ STATE_MAC_MAX_STATES ];
  /** Marked true when a state transition is detected. State
   *  transitions are detected in the event handler. */
  bool     transitionRequested;
  /** The next state requested. This is always updated in the
   *  state handler and either points to the current state or
   *  the next State when requested. */
  uint32_t nextStateRequested;
  /** Handle to module logger. Refer to Logable.h */
  StateMachineAllocate fnMachineAlloc;
  StateMachineInit fnMachineInit;
  StateMachineDealloc fnMachineDealloc;
  void *pPrivate;
  bool stateMachineStarted;
  Logable  logger;
}StateMachine;


StateMachine* StateMachineCreate( const StateMachineInitParams* pInit ); 
/**
 * State specific event handler 
 *  
 * The function will call the state specific event handler and 
 * will latch any state transition requested. 
 * 
 * 
 * @param pStateMac: StateMachine* - pointer to valid state 
 *                 machine that has been started.
 * @param pEvent: AppEvent* - pointer to valid Event 
 */
void StateMachineHandleEvent( StateMachine *pStateMac, void* pEvent );

/**
 * Register state with state machine. 
 *  
 * The function registers a fully allocated and initialized 
 * state with the state machine. Should be called before 
 * starting the state machine. 
 * 
 * 
 * @param pStateMac: StateMachine* - Pointer to valid state 
 *                 machine
 * @param pState: State* - Pointer to valid State 
 */
void StateMachineRegisterState( StateMachine *pStateMac, State* pState );
/** 
 * Get state for requested stateId from state machine 
 *   
 *  Returns the State* structure for the requested stateId
 *  (based on what was registered), from the provided state
 *  machine.
 *  
 * @param pStateMac: StateMachine* - pointer to valid state 
 *                 machine
 * @param stateId: uint32_t - valid state Id. 
 * 
 * @return State* - pointer to state structure requested or NULL
 *         if not found.
 */
State* StateMachineGetState( StateMachine* pStateMac, uint32_t stateId );
/**
 * Start the state machine. 
 *  
 * This will load the Inital state into the state machine and 
 * thus hook it up to the event handler. 
 * 
 * 
 * @param pStateMac: StateMachine* - pointer to valid state 
 *                 machine
 * @param initialState: uint32_t - valid initial state id.
 */
void StateMachineStart( StateMachine* pStateMac, uint32_t initialState );

/**
 * StateMachineStop - This will stop the state machine. All 
 * subsequent events posted will be discarded. Calling this is 
 * not so simple. You should call this in the same context that 
 * the State Machine Events are handled, or you have to 
 * synchronoize between the state machine event hander thread 
 * and the poster of this. This will effectively stop the state 
 * machine from engaging.  
 * 
 * 
 * @param pStateMac -  
 */
void StateMachineStop( StateMachine* pStateMac );
/**
 * Executes state transition.
 * 
 *  Will execute a state transition if one is requested. The
 *  function is called by the event handler when it is ready to
 *  execute a state transition.
 *  
 * @param pStateMac: StateMachine* - pointer to valid state 
 *                 machine.
 */
void StateMachineTransition( StateMachine *pStateMac );

#endif // __STATE_MACHINE_H__
