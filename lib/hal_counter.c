/**************************************************************************************************
 *                                            INCLUDES
 **************************************************************************************************/

#include "hal_counter.h"
#include "debug.h"
#include "OnBoard.h"
#include "ZComDef.h"

#include "hal_defs.h"
#include "hal_drivers.h"
#include "hal_mcu.h"

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/

#ifndef HAL_COUNTER_P0_INPUT_PINS
  #define HAL_COUNTER_P0_INPUT_PINS 0x00
#endif

#ifndef HAL_COUNTER_P1_INPUT_PINS
  #define HAL_COUNTER_P1_INPUT_PINS 0x00
#endif

#ifndef HAL_COUNTER_P2_INPUT_PINS
  #define HAL_COUNTER_P2_INPUT_PINS 0x00
#endif


#ifndef HAL_COUNTER_P0_INPUT_PINS_EDGE
  #define HAL_COUNTER_P0_INPUT_PINS_EDGE HAL_COUNTER_FALLING_EDGE
#endif

#ifndef HAL_COUNTER_P1_INPUT_PINS_EDGE
  #define HAL_COUNTER_P1_INPUT_PINS_EDGE HAL_COUNTER_FALLING_EDGE
#endif

#ifndef HAL_COUNTER_P2_INPUT_PINS_EDGE
  #define HAL_COUNTER_P2_INPUT_PINS_EDGE HAL_COUNTER_FALLING_EDGE
#endif

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/
// Task ID not initialized
#define NO_TASK_ID 0xFF

#define HAL_COUNTER_DEBOUNCE_VALUE   25

#define HAL_COUNTER_P0_EDGE_BITS     HAL_COUNTER_BIT0
#define HAL_COUNTER_P1_EDGE_BITS     (HAL_COUNTER_BIT1 | HAL_COUNTER_BIT2)
#define HAL_COUNTER_P2_EDGE_BITS     HAL_COUNTER_BIT3

/**************************************************************************************************
 *                                            TYPEDEFS
 **************************************************************************************************/

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
static uint8 TaskID  = NO_TASK_ID;
static uint8 registeredCountersTaskID = NO_TASK_ID;
/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
void halProcessCounterInterrupt ( uint8 portNum );

uint8 counter_state_send ( uint8 portNumber, uint8 pinNumber );

void zclCounter_Init ( uint8 task_id ) {
  TaskID = task_id;
}

void HalCounterInit ( void ) {
#if HAL_COUNTER_P0_INPUT_PINS
  P0SEL &= ~HAL_COUNTER_P0_INPUT_PINS;
  P0DIR &= ~(HAL_COUNTER_P0_INPUT_PINS);
#endif

#if HAL_COUNTER_P1_INPUT_PINS
  P1SEL &= ~HAL_COUNTER_P1_INPUT_PINS;
  P1DIR &= ~(HAL_COUNTER_P1_INPUT_PINS);
#endif

#if HAL_COUNTER_P2_INPUT_PINS
  P2SEL &= ~(HAL_COUNTER_P2_INPUT_PINS);
  P2DIR &= ~(HAL_COUNTER_P2_INPUT_PINS);
#endif
}

void HalCounterConfig ( void ) {
#if HAL_COUNTER_P0_INPUT_PINS 
  P0IEN |= HAL_COUNTER_P0_INPUT_PINS;
  IEN1 |= HAL_COUNTER_BIT5;            // enable port0 int
  P0INP &= ~HAL_COUNTER_P0_INPUT_PINS; // Pullup/pulldown

#if (HAL_COUNTER_P0_INPUT_PINS_EDGE == HAL_COUNTER_FALLING_EDGE)
  P2INP &= ~HAL_COUNTER_BIT5; // pull up
  MicroWait(50);
  PICTL |= HAL_COUNTER_P0_EDGE_BITS; // set falling edge on port
#else
  P2INP |= HAL_COUNTER_BIT5; // pull down
  MicroWait(50);
  PICTL &= ~(HAL_COUNTER_P0_EDGE_BITS);
#endif

#endif

#if HAL_COUNTER_P1_INPUT_PINS
  P1IEN |= HAL_COUNTER_P1_INPUT_PINS;
  IEN2 |= HAL_COUNTER_BIT4; // enable port1 int
  P1INP &= ~HAL_COUNTER_P1_INPUT_PINS; //Pullup/pulldown 
#if (HAL_COUNTER_P1_INPUT_PINS_EDGE == HAL_COUNTER_FALLING_EDGE)
  P2INP &= ~HAL_COUNTER_BIT6;        // pull up
  MicroWait(50);
  PICTL |= HAL_COUNTER_P1_EDGE_BITS; // set falling edge on port
#else
  P2INP |= HAL_COUNTER_BIT6; // pull down
  MicroWait(50);
  PICTL &= ~HAL_COUNTER_P1_EDGE_BITS;
#endif

#endif

#if HAL_COUNTER_P2_INPUT_PINS
  P2SEL &= ~(HAL_COUNTER_P2_INPUT_PINS);
  P2DIR &= ~(HAL_COUNTER_P2_INPUT_PINS);
  
  P2IEN |= HAL_COUNTER_P2_INPUT_PINS;
  IEN2 |= HAL_COUNTER_BIT1; // enable port2 int
  P2INP &= ~(HAL_COUNTER_P2_INPUT_PINS); //Pullup/pulldown
#if (HAL_COUNTER_P2_INPUT_PINS_EDGE == HAL_COUNTER_FALLING_EDGE)
  P2INP &= ~HAL_COUNTER_BIT7;        // pull up
  MicroWait(50);
  PICTL |= HAL_COUNTER_P2_EDGE_BITS; // set falling edge on port
#else
  P2INP |= HAL_COUNTER_BIT7; // pull down
  MicroWait(50);
  PICTL &= ~HAL_COUNTER_P2_EDGE_BITS;
#endif

#endif
}

uint8 RegisterForCounters ( uint8 task_id ) {
  if ( registeredCountersTaskID == NO_TASK_ID ) {
    registeredCountersTaskID = task_id;
    return ( true );
  }
  else {
    return ( false );
  }
}

void halProcessCounterInterrupt ( uint8 portNumber ) {
  uint16 event_id = 0;
  uint8 enabledPins;
  uint8 PNIFG;
  
  switch ( portNumber ) {
    case HAL_COUNTER_PORT0:
      enabledPins = HAL_COUNTER_P0_INPUT_PINS;
      PNIFG = P0IFG;
      event_id |= HAL_COUNTER_PORT0 << 8;
      break;
    case HAL_COUNTER_PORT1:
      enabledPins = HAL_COUNTER_P1_INPUT_PINS;
      PNIFG = P1IFG;
      event_id |= HAL_COUNTER_PORT1 << 8;
      break;
    case HAL_COUNTER_PORT2:
      enabledPins = HAL_COUNTER_P2_INPUT_PINS;
      PNIFG = P2IFG;
      event_id |= HAL_COUNTER_PORT2 << 8;
      break;
    default:
      return;
  }
  
  for (uint8 pinIndex = 0; pinIndex < 8; pinIndex++) {
    uint8 pinNumber = 1 << pinIndex;
    
    if ( (enabledPins & pinNumber) && (PNIFG & pinNumber) ) {
      osal_start_timerEx ( TaskID, event_id | pinNumber, HAL_COUNTER_DEBOUNCE_VALUE );
    }
  }
}

uint16 zclCounter_event_loop ( uint8 task_id, UINT16 events ) {
  uint8 enabledPins;
  uint8 portsNumber = (events >> 8);
  LREP("[zclCounter_event_loop]: events: %d\r\n", events);
  
  for (uint8 portIndex = 0; portIndex < 3; portIndex++) {
    uint8 portNumber = 1 << portIndex;

    if ( (portsNumber &  portNumber) ) {
      uint8 pinsNumber = events & ~(portNumber << 8);

      switch ( portIndex ) {
        case 0:
          enabledPins = HAL_COUNTER_P0_INPUT_PINS;
          break;
        case 1:
          enabledPins = HAL_COUNTER_P1_INPUT_PINS;
          break;
        case 2:
          enabledPins = HAL_COUNTER_P2_INPUT_PINS;
          break;
        default:
          continue;
      }

      for (uint8 pinIndex = 0; pinIndex < 8; pinIndex++) {
        uint8 pinNumber = 1 << pinIndex;
        if ( (enabledPins & pinNumber) && (pinsNumber & pinNumber) ) {
          counter_state_send ( portIndex, pinIndex );
        }
      }
    }
  }

  return 0;
}

uint8 counter_state_send ( uint8 port, uint8 pin ) { 
  if ( registeredCountersTaskID == NO_TASK_ID ) {
    return ( ZFailure );
  }
  
  counterChange_t *msgPtr;
  // Send the address to the task
  msgPtr = (counterChange_t *)osal_msg_allocate( sizeof(counterChange_t) );
  if ( msgPtr )
  {
    msgPtr->hdr.event = COUNTER_CHANGE;
    msgPtr->port = port;
    msgPtr->pin = pin;

    osal_msg_send( registeredCountersTaskID, (uint8 *)msgPtr );
  }
  return ( ZSuccess );
}

#if HAL_COUNTER_P0_INPUT_PINS
HAL_ISR_FUNCTION(halKeyPort0Isr, P0INT_VECTOR) {
    HAL_ENTER_ISR();

    if (P0IFG & HAL_COUNTER_P0_INPUT_PINS) {
        halProcessCounterInterrupt(HAL_COUNTER_PORT0);
    }

    P0IFG = 0;
    P0IF = 0;

    CLEAR_SLEEP_MODE();
    HAL_EXIT_ISR();
}
#endif

#if HAL_COUNTER_P1_INPUT_PINS
HAL_ISR_FUNCTION(halKeyPort1Isr, P1INT_VECTOR) {
    HAL_ENTER_ISR();

    if (P1IFG & HAL_COUNTER_P1_INPUT_PINS) {
        halProcessCounterInterrupt(HAL_COUNTER_PORT1);
    }

    P1IFG = 0;
    P1IF = 0;

    CLEAR_SLEEP_MODE();
    HAL_EXIT_ISR();
}
#endif

#if HAL_COUNTER_P2_INPUT_PINS
HAL_ISR_FUNCTION(halKeyPort2Isr, P2INT_VECTOR) {
    HAL_ENTER_ISR();

    if (P2IFG & HAL_COUNTER_P2_INPUT_PINS) {
        halProcessCounterInterrupt(HAL_COUNTER_PORT2);
    }

    P2IFG = 0;
    P2IF = 0;

    CLEAR_SLEEP_MODE();
    HAL_EXIT_ISR();
}
#endif