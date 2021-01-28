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

#if defined(HAL_BOARD_TARGET)
//    #define HAL_KEY_P0_INPUT_PINS BV(1)
//    #define HAL_KEY_P1_INPUT_PINS (BV(2) | BV(3))
    #define CO2_UART_PORT 0x00
    #define HAL_UART_DMA 1
    #define HAL_UART_ISR 0
    #define INT_HEAP_LEN 2256
#elif defined(HAL_BOARD_CHDTECH_DEV)
    #define HAL_UART_DMA 1
    #define HAL_UART_ISR 2
    #define CO2_UART_PORT  0x01

    #define HAL_RESET_BUTTON_SBIT     BV(1)
    #define HAL_HOT_COUNTER_SBIT      BV(2)
    #define HAL_COLD_COUNTER_SBIT     BV(3)

    #define HAL_HOT_KEY_PORT          HAL_KEY_PORT1
    #define HAL_COLD_KEY_PORT         HAL_KEY_PORT1
    #define HAL_RESET_BUTTON_KEY_PORT HAL_KEY_PORT0

    #define HAL_KEY_P0_INPUT_PINS     HAL_RESET_BUTTON_SBIT
    #define HAL_KEY_P1_INPUT_PINS     HAL_HOT_COUNTER_SBIT | HAL_COLD_COUNTER_SBIT

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
