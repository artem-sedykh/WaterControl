#ifndef HAL_COUNTER_H
#define HAL_COUNTER_H

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************************************
 *                                             INCLUDES
 **************************************************************************************************/
#include "osal.h"
#include "ZComDef.h"
  
/**************************************************************************************************
 * MACROS
 **************************************************************************************************/

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/
#define COUNTER_CHANGE                      0xCA
  
//#define HAL_COUNTER_EVENT2                  0x0400
//#define HAL_COUNTER_EVENT1                  0x0200
//#define HAL_COUNTER_EVENT0                  0x0100

#define HAL_COUNTER_BIT0   0x01
#define HAL_COUNTER_BIT1   0x02
#define HAL_COUNTER_BIT2   0x04
#define HAL_COUNTER_BIT3   0x08
#define HAL_COUNTER_BIT4   0x10
#define HAL_COUNTER_BIT5   0x20
#define HAL_COUNTER_BIT6   0x40
#define HAL_COUNTER_BIT7   0x80

#define HAL_COUNTER_RISING_EDGE  0
#define HAL_COUNTER_FALLING_EDGE 1

#define HAL_COUNTER_PORT0  0x01
#define HAL_COUNTER_PORT1  0x02
#define HAL_COUNTER_PORT2  0x04

/**************************************************************************************************
 * TYPEDEFS
 **************************************************************************************************/
//typedef void (*halCounterCBack_t)( uint8 portNumber, uint8 pinNumber, bool isPressed );

typedef struct
{
  osal_event_hdr_t hdr;
  bool state;
  uint8 port; 
  uint8 pin;
} counterChange_t;

/**************************************************************************************************
 *                                             GLOBAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 *                                             FUNCTIONS - API
 **************************************************************************************************/

/*
 * Initialize the Counter Service
 */
extern void zclCounter_Init ( uint8 task_id );

extern void HalCounterConfig ( void );

extern uint16 zclCounter_event_loop ( uint8 task_id, UINT16 events );

extern uint8 RegisterForCounters ( uint8 task_id );

/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
