#define TC_LINKKEY_JOIN
#define NV_INIT
#define NV_RESTORE

#define TP2_LEGACY_ZC
// patch sdk
// #define ZDSECMGR_TC_ATTEMPT_DEFAULT_KEY TRUE

#define NWK_AUTO_POLL
#define MULTICAST_ENABLED FALSE

#define ZCL_READ
#define ZCL_WRITE
#define ZCL_BASIC
#define ZCL_IDENTIFY
#define ZCL_SCENES
#define ZCL_GROUPS
#define ZCL_ON_OFF


#define ZCL_REPORTING_DEVICE

#define BDBREPORTING_MAX_ANALOG_ATTR_SIZE  4
#define BDB_MAX_CLUSTERENDPOINTS_REPORTING 2
#define BDBREPORTING_DEFAULTCHANGEVALUE {0x00, 0x00, 0x00, 0x00}

#define DISABLE_GREENPOWER_BASIC_PROXY
#define BDB_FINDING_BINDING_CAPABILITY_ENABLED 1
#define BDB_REPORTING TRUE

#define HAL_BUZZER FALSE
#define HAL_KEY TRUE
#define ISR_KEYINTERRUPT
#define HAL_COUNTER TRUE

#define HAL_LED TRUE
#define HAL_ADC FALSE
#define HAL_LCD FALSE

#define BLINK_LEDS TRUE

// one of this boards
// #define HAL_BOARD_TARGET
// #define HAL_BOARD_CHDTECH_DEV

#if !defined(HAL_BOARD_TARGET) && !defined(HAL_BOARD_CHDTECH_DEV)
#error "Board type must be defined"
#endif

// main button
#define HAL_KEY_P2_INPUT_PINS         BV(0)
// C1 COUNTER PORT: P0_0
#define HAL_C1_PORT_NUMBER            0
#define HAL_C1_PIN_NUMBER             0
// C2 COUNTER PORT: P0_1
#define HAL_C2_PORT_NUMBER            0
#define HAL_C2_PIN_NUMBER             1
// C3 COUNTER PORT: P0_2
#define HAL_C3_PORT_NUMBER            0
#define HAL_C3_PIN_NUMBER             2
// C4 COUNTER PORT: P0_4
#define HAL_C4_PORT_NUMBER            0
#define HAL_C4_PIN_NUMBER             4

#define HAL_COUNTER_P0_INPUT_PINS     BV(HAL_C1_PIN_NUMBER) | BV(HAL_C2_PIN_NUMBER) | BV(HAL_C3_PIN_NUMBER) | BV(HAL_C4_PIN_NUMBER)

#if defined(HAL_BOARD_TARGET)
    #define HAL_UART_DMA 1
    #define HAL_UART_ISR 0
    #define INT_HEAP_LEN 2256
#elif defined(HAL_BOARD_CHDTECH_DEV)
    #define HAL_UART_DMA 1
    #define HAL_UART_ISR 2
    #define DO_DEBUG_UART
#endif

#define FACTORY_RESET_HOLD_TIME_LONG 5000

#ifdef DO_DEBUG_UART
    #define HAL_UART TRUE
    #define HAL_UART_DMA 1
    #define INT_HEAP_LEN 2060
#endif

#define HAL_UART TRUE

#include "hal_board_cfg.h"
