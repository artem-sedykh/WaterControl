#ifndef ZCL_WATERCONTROL_H
#define ZCL_WATERCONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "uint48.h"
/*********************************************************************
 * CONSTANTS
 */
#define ENDPOINTS_COUNT       2
#define ENDPOINT_ATTRS_COUNT  18 
#define APP_REPORT_DELAY     ((uint32)5 * (uint32)1000) // 5 sec
#define APP_SAVE_ATTRS_DELAY ((uint32)600 * (uint32)1000) // 10 minute

#define NW_HOT_CONFIG                   0x0402
#define NW_COLD_CONFIG                  0x0412

   
// Application Events
#define APP_REPORT_EVT                  0x0001
#define APP_SAVE_ATTRS_EVT              0x0002
#define APP_PUSH_STATE_EVT              0x0004


#define CID_BASIC                       ZCL_CLUSTER_ID_GEN_BASIC
#define CID_GROUPS                      ZCL_CLUSTER_ID_GEN_GROUPS
#define CID_ON_OFF                      ZCL_CLUSTER_ID_GEN_ON_OFF
//***************************Metering******************************
#define CID_METERING                    ZCL_CLUSTER_ID_SE_METERING
//***************************Attributes****************************
#define ATTRID_CURRENT_SUMM_DELIVERED   0x0000
#define ATTRID_STATUS                   0x0200
#define ATTRID_UNIT_OF_MEASURE          0x0300
#define ATTRID_MULTIPLIER               0x0301
#define ATTRID_DIVISOR                  0x0302
#define ATTRID_SUMM_FORMATTING          0x0303
#define ATTRID_METERING_DEVICE_TYPE     0x0306
//*****************************************************************
#define ZCL_UINT8                       ZCL_DATATYPE_UINT8
#define ZCL_UINT16                      ZCL_DATATYPE_UINT16
#define ZCL_UINT24                      ZCL_DATATYPE_UINT24
#define ZCL_UINT48                      ZCL_DATATYPE_UINT48
#define ZCL_CHAR_STR                    ZCL_DATATYPE_CHAR_STR
#define ZCL_ENUM8                       ZCL_DATATYPE_ENUM8
#define ZCL_BOOLEAN                     ZCL_DATATYPE_BOOLEAN
#define ZCL_BITMAP8                     ZCL_DATATYPE_BITMAP8

/*********************************************************************
 * MACROS
 */
#define R                                    (ACCESS_CONTROL_READ)
#define RW                                   (R | ACCESS_CONTROL_WRITE | ACCESS_CONTROL_AUTH_WRITE)
#define RR                                   (ACCESS_CONTROL_READ | ACCESS_REPORTABLE)
#define RRW                                  (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CONTROL_AUTH_WRITE | ACCESS_REPORTABLE)

#define DECLARE_APPLY_RELAY(INDEX)           static void ApplyRelay_EP##INDEX (uint8 state ){ HAL_APPLY_RELAY##INDEX(state); }
#define DECLARE_BASIC_RESETCB(INDEX)         static void zclWaterControl_BasicResetCB_EP##INDEX (void ){ zclWaterControl_BasicResetCB ( &zcl_Configs[INDEX] ); }
#define DECLARE_ON_OFFCB(INDEX)              static void zclWaterControl_OnOffCB_EP##INDEX (uint8 cmd ){ zclWaterControl_OnOffCB ( &zcl_Configs[INDEX], cmd ); }
#define GET_FUNC(NAME, INDEX)                NAME## _EP ##INDEX
/*********************************************************************
 * TYPEDEFS
 */

typedef struct {

/*CurrentSummationDelivered represents the most recent summed value of Energy, Gas, or Water delivered
  and consumed in the premises. CurrentSummationDelivered is mandatory and must be provided as part of the
  minimum data set to be provided by the metering device. CurrentSummationDelivered is updated continuously as new measurements are made. */
  uint48 CurrentSummDelivered;

/*Divisor provides a value to divide the results of applying the Multiplier Attribute against a raw or uncompensated sensor count of Energy, Gas, or Water being measured by the metering device. If present, this
  attribute must be applied against all summation, consumption and demand values to derive the delivered
  and received values expressed in the unit of measure specified. This attribute must be used in conjunction
  with the Multiplier attribute. */
  uint24 Divisor;

/*Multiplier provides a value to be multiplied against a raw or uncompensated sensor count of Energy, Gas, or
  Water being measured by the metering device. If present, this attribute must be applied against all summation, consumption and demand values to derive the delivered and received values expressed in the unit of
  measure specified. This attribute must be used in conjunction with the Divisor attribute. */
  uint24 Multiplier;

/*Bit 7    Bit 6        Bit 5   Bit 4      Bit 3   Bit 2   Bit 1    Bit 0
  Reverse  Service      Leak    Low        Pipe    Tamper  Low      Check
  Flow     Disconnect   Detect  Pressure   Empty   Detect  Battery  Meter

  The definitions of the Water Status bits are:
  Reverse Flow      : Set to true if flow detected in the opposite direction to normal (from consumer to supplier).
  Service Disconnect: Set to true when the service has been disconnected to this premises. Ex. The valve is in the closed position preventing delivery of water.
  Leak Detect       : Set to true when a leak has been detected.
  Low Pressure      : Set to true when the pressure at the meter is below the meter's low pressure threshold value.
  Pipe Empty        : Set to true when the service pipe at the meter is empty and there is no flow in either direction.
  Tamper Detect     : Set to true if a tamper event has been detected.
  Low Battery       : Set to true when the battery needs maintenance.
  Check Meter       : Set to true when a non fatal problem has been detected on the meter such as a measurement error, memory error, or self check error. */
  uint8  Status;

/*SummationFormatting provides a method to properly decipher the number of digits and the decimal location
  of the values found in the Summation Information Set of attributes. This attribute is to be decoded as follows:

  Bits 0 to 2: Number of Digits to the right of the Decimal Point.
  Bits 3 to 6: Number of Digits to the left of the Decimal Point.
  Bit 7: If set, suppress leading zeros. */
  uint8  SummFormatting;

/*Relay status: 0 - OFF, 1 - ON */
  uint8  RelayState;
} endpoint_config_t;

typedef struct {
  endpoint_config_t Config;
  bool              Changed;
  uint16            NVkey;
  uint8             Endpoint;
  bool              ReportCurrentSummDelivered;
  void (*ApplyRelay) (uint8);
} app_config_t;
     
/*********************************************************************
 * VARIABLES
 */

extern const uint8               zcl_EndpointsCount;
extern app_config_t              zcl_Configs[ENDPOINTS_COUNT];

extern SimpleDescriptionFormat_t zclEndpoints[];
extern CONST zclAttrRec_t        zclEndpoints_Attrs[][ENDPOINT_ATTRS_COUNT];
extern const uint8               zclEndpoint_AttrsCount;

/*********************************************************************
 * PROTOTYPES
 */

extern void   zclWaterControl_Init                           ( byte task_id );
extern uint16 zclWaterControl_event_loop                     ( byte task_id, UINT16 events );
extern void   zclWaterControl_ResetAttributesToDefaultValues ( app_config_t *config );
extern void   zclWaterControl_InitClusters                   ( void );

#ifdef __cplusplus
}
#endif

#endif /* ZCL_WATERCONTROL_H */
