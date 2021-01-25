#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"

#include "version.h"
#include "zcl_watercontrol.h"
#include "zcl_watercontrol_data.h"

const cId_t zclHotEndpoint_InClusterList[]  = { CID_BASIC, CID_GROUPS, CID_ON_OFF };
const cId_t zclHotEndpoint_OutClusterList[] = { CID_METERING };

#define APP_MAX_INCLUSTERS   (sizeof(zclHotEndpoint_InClusterList) / sizeof(zclHotEndpoint_InClusterList[0]))
#define APP_MAX_OUTCLUSTERS  (sizeof(zclHotEndpoint_OutClusterList) / sizeof(zclHotEndpoint_OutClusterList[0]))

CONST zclAttrRec_t zclHotEndpoint_Attrs[] = {
  /*                                               ZCL_CLUSTER_ID_GEN_BASIC                                                            */
  { CID_BASIC    ,  { ATTRID_BASIC_ZCL_VERSION       , ZCL_UINT8          , R  , (void *)&zclWaterControl_ZCLVersion                  } },
  { CID_BASIC    ,  { ATTRID_BASIC_APPL_VERSION      , ZCL_UINT8          , R  , (void *)&zclWaterControl_ApplicationVersion          } },
  { CID_BASIC    ,  { ATTRID_BASIC_STACK_VERSION     , ZCL_UINT8          , R  , (void *)&zclWaterControl_StackVersion                } },
  { CID_BASIC    ,  { ATTRID_BASIC_HW_VERSION        , ZCL_UINT8          , R  , (void *)&zclWaterControl_HWRevision                  } },
  { CID_BASIC    ,  { ATTRID_BASIC_MANUFACTURER_NAME , ZCL_CHAR_STR       , R  , (void *)zclWaterControl_ManufacturerName             } },
  { CID_BASIC    ,  { ATTRID_BASIC_MODEL_ID          , ZCL_CHAR_STR       , R  , (void *)zclWaterControl_ModelId                      } },
  { CID_BASIC    ,  { ATTRID_BASIC_DATE_CODE         , ZCL_CHAR_STR       , R  , (void *)zclWaterControl_DateCode                     } },
  { CID_BASIC    ,  { ATTRID_BASIC_POWER_SOURCE      , ZCL_ENUM8          , R  , (void *)&zclWaterControl_PowerSource                 } },
  { CID_BASIC    ,  { ATTRID_BASIC_SW_BUILD_ID       , ZCL_CHAR_STR       , R  , (void *)zclWaterControl_DateCode                     } },
  { CID_BASIC    ,  { ATTRID_CLUSTER_REVISION        , ZCL_UINT16         , R  , (void *)&zclWaterControl_clusterRevision_all         } },
  /*                                               ZCL_CLUSTER_ID_GEN_ON_OFF                                                           */
  { CID_ON_OFF   , { ATTRID_ON_OFF                   , ZCL_BOOLEAN        , RR , (void *)&zcl_Configs[0].Config.RelayState            } },
  /*                                               ZCL_CLUSTER_ID_SE_METERING                                                          */
  { CID_METERING , { ATTRID_CURRENT_SUMM_DELIVERED   , ZCL_UINT48         , RRW, (void *)&zcl_Configs[0].Config.CurrentSummDelivered  } },
  { CID_METERING , { ATTRID_STATUS                   , ZCL_BITMAP8        , R  , (void *)&zcl_Configs[0].Config.Status                } },
  { CID_METERING , { ATTRID_UNIT_OF_MEASURE          , ZCL_ENUM8          , R  , (void *)&zclWaterControl_UnitofMeasure               } },
  { CID_METERING , { ATTRID_MULTIPLIER               , ZCL_UINT24         , RW , (void *)&zcl_Configs[0].Config.Multiplier            } },
  { CID_METERING , { ATTRID_DIVISOR                  , ZCL_UINT24         , RW , (void *)&zcl_Configs[0].Config.Divisor               } },  
  { CID_METERING , { ATTRID_SUMM_FORMATTING          , ZCL_BITMAP8        , R  , (void *)&zcl_Configs[0].Config.SummFormatting        } },
  { CID_METERING , { ATTRID_METERING_DEVICE_TYPE     , ZCL_BITMAP8        , R  , (void *)&zclWaterControl_MeteringDeviceType          } },
};

uint8 const zclHotEndpoint_AttrsCount = (sizeof(zclHotEndpoint_Attrs) / sizeof(zclHotEndpoint_Attrs[0]));

SimpleDescriptionFormat_t zclHotEndpoint = {
  1,                                                          //  int Endpoint;
  ZCL_HA_PROFILE_ID,                                          //  uint16 AppProfId;
  ZCL_HA_DEVICEID_ON_OFF_SWITCH,                              //  uint16 AppDeviceId; 
  APP_DEVICE_VERSION,                                         //  int   AppDevVer:4;
  APP_FLAGS,                                                  //  int   AppFlags:4;
  APP_MAX_INCLUSTERS,                                         //  byte  AppNumInClusters;
  (cId_t *)zclHotEndpoint_InClusterList,                      //  byte *pAppInClusterList;
  APP_MAX_OUTCLUSTERS,                                        //  byte  AppNumInClusters;
  (cId_t *)zclHotEndpoint_OutClusterList                      //  byte *pAppInClusterList;
};