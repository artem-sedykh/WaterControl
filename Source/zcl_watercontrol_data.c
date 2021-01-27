#include "zcl_watercontrol.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "version.h"
#include "hal_board.h"

#define APP_DEVICE_VERSION      1
#define APP_FLAGS               0

#define APP_HWVERSION           1
#define APP_ZCLVERSION          1

#define DEFAULT_DIVISOR         1000
#define DEFAULT_MULTIPLIER      1
#define DEFAULT_STATUS          0
/* 011  [3]    - Number of Digits to the right of the Decimal Point
   1111 [15]   - Number of Digits to the left of the Decimal Point
   1    [True] - Suppress leading zeros */
#define DEFAULT_SUMM_FORMATTING 251 //0b11111011
#define DEFAULT_RELAY_STATE     1

// Global attributes
const uint16 zclWaterControl_clusterRevision_all = 0x0001; 

const uint8  zclWaterControl_HWRevision          = APP_HWVERSION;
const uint8  zclWaterControl_ZCLVersion          = APP_ZCLVERSION;
const uint8  zclWaterControl_ApplicationVersion  = 4;
const uint8  zclWaterControl_StackVersion        = 4;

const uint8  zclWaterControl_ManufacturerName[]  = { 6, 'T','e','a','m','4','2' };
const uint8  zclWaterControl_ModelId[]           = { 4, 'W','C','0','3' };
const uint8  zclWaterControl_PowerSource         = POWER_SOURCE_MAINS_1_PHASE;

const uint8  zclWaterControl_MeteringDeviceType  = 2;  // Water Metering see https://zigbeealliance.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf
const uint8  zclWaterControl_UnitofMeasure       = 1;  //m3 (Cubic Meter) & m3/h (Cubic Meter per Hour) in pure binary format see https://zigbeealliance.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf

const uint8  zcl_EndpointsCount                  = ENDPOINTS_COUNT;

DECLARE_APPLY_RELAY(0)
DECLARE_APPLY_RELAY(1)

app_config_t zcl_Configs[ENDPOINTS_COUNT] = {
  { { {0, 0, 0, 0, 0, 0}, DEFAULT_DIVISOR, DEFAULT_MULTIPLIER, DEFAULT_STATUS, DEFAULT_SUMM_FORMATTING, DEFAULT_RELAY_STATE }, FALSE, NW_HOT_CONFIG,  1, TRUE, GET_FUNC(ApplyRelay, 0) },
  { { {0, 0, 0, 0, 0, 0}, DEFAULT_DIVISOR, DEFAULT_MULTIPLIER, DEFAULT_STATUS, DEFAULT_SUMM_FORMATTING, DEFAULT_RELAY_STATE }, FALSE, NW_COLD_CONFIG, 2, TRUE, GET_FUNC(ApplyRelay, 1) }
};

const cId_t zclEndpoint_InClusterList[]  = { CID_BASIC, CID_GROUPS, CID_ON_OFF };
const cId_t zclEndpoint_OutClusterList[] = { CID_METERING };

#define ZCL_ENDPOINT_ATTRS(INDEX)                                                                                                                   \
  {                                                                                                                                                 \
    { CID_BASIC    ,  { ATTRID_BASIC_ZCL_VERSION       , ZCL_UINT8          , R  , (void *)&zclWaterControl_ZCLVersion                      } },    \
    { CID_BASIC    ,  { ATTRID_BASIC_APPL_VERSION      , ZCL_UINT8          , R  , (void *)&zclWaterControl_ApplicationVersion              } },    \
    { CID_BASIC    ,  { ATTRID_BASIC_STACK_VERSION     , ZCL_UINT8          , R  , (void *)&zclWaterControl_StackVersion                    } },    \
    { CID_BASIC    ,  { ATTRID_BASIC_HW_VERSION        , ZCL_UINT8          , R  , (void *)&zclWaterControl_HWRevision                      } },    \
    { CID_BASIC    ,  { ATTRID_BASIC_MANUFACTURER_NAME , ZCL_CHAR_STR       , R  , (void *)zclWaterControl_ManufacturerName                 } },    \
    { CID_BASIC    ,  { ATTRID_BASIC_MODEL_ID          , ZCL_CHAR_STR       , R  , (void *)zclWaterControl_ModelId                          } },    \
    { CID_BASIC    ,  { ATTRID_BASIC_DATE_CODE         , ZCL_CHAR_STR       , R  , (void *)zclWaterControl_DateCode                         } },    \
    { CID_BASIC    ,  { ATTRID_BASIC_POWER_SOURCE      , ZCL_ENUM8          , R  , (void *)&zclWaterControl_PowerSource                     } },    \
    { CID_BASIC    ,  { ATTRID_BASIC_SW_BUILD_ID       , ZCL_CHAR_STR       , R  , (void *)zclWaterControl_DateCode                         } },    \
    { CID_BASIC    ,  { ATTRID_CLUSTER_REVISION        , ZCL_UINT16         , R  , (void *)&zclWaterControl_clusterRevision_all             } },    \
    { CID_ON_OFF   ,  { ATTRID_ON_OFF                  , ZCL_BOOLEAN        , RR , (void *)&(zcl_Configs[INDEX].Config.RelayState)          } },    \
    { CID_METERING ,  { ATTRID_CURRENT_SUMM_DELIVERED  , ZCL_UINT48         , RW , (void *)&zcl_Configs[INDEX].Config.CurrentSummDelivered  } },    \
    { CID_METERING ,  { ATTRID_STATUS                  , ZCL_BITMAP8        , R  , (void *)&zcl_Configs[INDEX].Config.Status                } },    \
    { CID_METERING ,  { ATTRID_UNIT_OF_MEASURE         , ZCL_ENUM8          , R  , (void *)&zclWaterControl_UnitofMeasure                   } },    \
    { CID_METERING ,  { ATTRID_MULTIPLIER              , ZCL_UINT24         , RW , (void *)&zcl_Configs[INDEX].Config.Multiplier            } },    \
    { CID_METERING ,  { ATTRID_DIVISOR                 , ZCL_UINT24         , RW , (void *)&zcl_Configs[INDEX].Config.Divisor               } },    \
    { CID_METERING ,  { ATTRID_SUMM_FORMATTING         , ZCL_BITMAP8        , R  , (void *)&zcl_Configs[INDEX].Config.SummFormatting        } },    \
    { CID_METERING ,  { ATTRID_METERING_DEVICE_TYPE    , ZCL_BITMAP8        , R  , (void *)&zclWaterControl_MeteringDeviceType              } },    \
  }                                                                                                                                                 \

CONST zclAttrRec_t zclEndpoints_Attrs[][ENDPOINT_ATTRS_COUNT] = { ZCL_ENDPOINT_ATTRS(0), ZCL_ENDPOINT_ATTRS(1) };

#define APP_MAX_INCLUSTERS   (sizeof(zclEndpoint_InClusterList) / sizeof(zclEndpoint_InClusterList[0]))
#define APP_MAX_OUTCLUSTERS  (sizeof(zclEndpoint_OutClusterList) / sizeof(zclEndpoint_OutClusterList[0]))

SimpleDescriptionFormat_t zclEndpoints[ENDPOINTS_COUNT]; 

uint8 const zclEndpoint_AttrsCount = ENDPOINT_ATTRS_COUNT;

void zclWaterControl_InitClusters ( void ) {
  uint8 i = 0;

  for (i = 0; i < zcl_EndpointsCount; ++i) {
    uint8 endpoint = i + 1;

    zclEndpoints[i].EndPoint           = endpoint;
    zclEndpoints[i].AppProfId          = ZCL_HA_PROFILE_ID;
    zclEndpoints[i].AppDeviceId        = ZCL_HA_DEVICEID_ON_OFF_SWITCH;
    zclEndpoints[i].AppDevVer          = APP_DEVICE_VERSION;
    zclEndpoints[i].Reserved           = APP_FLAGS;
    zclEndpoints[i].AppNumInClusters   = APP_MAX_INCLUSTERS;
    zclEndpoints[i].pAppInClusterList  = (cId_t *)zclEndpoint_InClusterList;
    
    zclEndpoints[i].AppNumOutClusters  = APP_MAX_OUTCLUSTERS;
    zclEndpoints[i].pAppOutClusterList = (cId_t *)zclEndpoint_OutClusterList;
  }
}

void zclWaterControl_ResetAttributesToDefaultValues ( app_config_t *config ) {
  config->Config.Divisor        = DEFAULT_DIVISOR;
  config->Config.Multiplier     = DEFAULT_MULTIPLIER;
  config->Config.Status         = DEFAULT_STATUS;
  config->Config.SummFormatting = DEFAULT_SUMM_FORMATTING;
  config->Config.RelayState     = DEFAULT_RELAY_STATE;
  
  for ( uint8 i = 0; i < 6; ++i ) {
    config->Config.CurrentSummDelivered.data[i] = 0;
  }
}
