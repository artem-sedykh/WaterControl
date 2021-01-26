#include <stdlib.h>

#include "ZComDef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "MT_SYS.h"

#include "nwk_util.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_diagnostic.h"
#include "zcl_watercontrol.h"

#include "bdb.h"
#include "bdb_interface.h"
#include "bdb_touchlink.h"
#include "bdb_touchlink_target.h"

#include "gp_interface.h"

#include "debug.h"
#include "onboard.h"
#include "commissioning.h"
#include "factory_reset.h"

/* HAL */
#include "hal_drivers.h"
#include "hal_led.h"
#include "hal_key.h"
#include "version.h"
#include "uint48.h"

/*********************************************************************
* MACROS
*/


/*********************************************************************
* CONSTANTS
*/


/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/
byte zclWaterControl_TaskID;


/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* LOCAL VARIABLES
*/
afAddrType_t inderect_DstAddr = {.addrMode = (afAddrMode_t)AddrNotPresent, .endPoint = 0, .addr.shortAddr = 0};

/*********************************************************************
* LOCAL FUNCTIONS
*/
static void      zclWaterControl_PushState         ( void );
static void      zclCommissioning_OnConnectCB      ( void );
static void      zclWaterControl_WriteAttrDataCB   ( uint8 endpoint, zclAttrRec_t *pAttr );
static void      zclWaterControl_SaveAttributes    ( void );
static void      zclWaterControl_RestoreAttributes ( void );

static void      zclWaterControl_BasicResetCB      ( app_config_t *config );
static void      zclHot_BasicResetCB               ( void );
static void      zclCold_BasicResetCB              ( void );

static void      zclWaterControl_OnOffCB           ( app_config_t *config, uint8 cmd );
static void      zclHot_OnOffCB                    ( uint8 );
static void      zclCold_OnOffCB                   ( uint8 );

static void      zclWaterControl_ApplyRelay        ( app_config_t *config );
static void      zclWaterControl_Increment         ( app_config_t *config );

static void      zclWaterControl_Report            ( void );

static void      zclWaterControl_HandleKeys        ( byte portAndAction, byte keyCode );

static zclGeneral_AppCallbacks_t zclHotEndpoint_CmdCallbacks  = {
  zclHot_BasicResetCB,
  NULL,
  zclHot_OnOffCB,
  NULL,                // On/Off cluster enhanced command Off with Effect
  NULL,                // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                // On/Off cluster enhanced command On with Timed Off
  NULL,                // RSSI Location command
  NULL                 // RSSI Location Response command
};

static zclGeneral_AppCallbacks_t zclColdEndpoint_CmdCallbacks = {
  zclCold_BasicResetCB,
  NULL, 
  zclCold_OnOffCB,
  NULL,                // On/Off cluster enhanced command Off with Effect
  NULL,                // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                // On/Off cluster enhanced command On with Timed Off
  NULL,                // RSSI Location command
  NULL                 // RSSI Location Response command
};

void zclWaterControl_Init( byte task_id )
{
  HalLedSet(HAL_LED_ALL, HAL_LED_MODE_BLINK);
  
  zclWaterControl_TaskID = task_id;

  zclWaterControl_RestoreAttributes ();
  
  bdb_RegisterSimpleDescriptor( &zclHotEndpoint );
  
  zclGeneral_RegisterCmdCallbacks( zclHotEndpoint.EndPoint, &zclHotEndpoint_CmdCallbacks );
  
  zcl_registerAttrList ( zclHotEndpoint.EndPoint, zclHotEndpoint_AttrsCount, zclHotEndpoint_Attrs );
  
  //--------------------------------------------------------------------------------------------------
  
  bdb_RegisterSimpleDescriptor( &zclColdEndpoint );
 
  zclGeneral_RegisterCmdCallbacks( zclColdEndpoint.EndPoint, &zclColdEndpoint_CmdCallbacks );
  
  zcl_registerAttrList ( zclColdEndpoint.EndPoint, zclColdEndpoint_AttrsCount, zclColdEndpoint_Attrs );
  
  //--------------------------------------------------------------------------------------------------
  
  zcl_registerWriteAttrDataCB ( zclWaterControl_WriteAttrDataCB );

  zclCommissioningRegisterOnConnectCB ( zclCommissioning_OnConnectCB );

  zcl_registerForMsg( zclWaterControl_TaskID );
  
  RegisterForKeys( zclWaterControl_TaskID );
  
  LREP("Build %s \r\n", zclWaterControl_DateCodeNT);
  
  osal_start_reload_timer ( zclWaterControl_TaskID, APP_REPORT_EVT, APP_REPORT_DELAY );

  //create loop for save attributes to nv
  osal_start_reload_timer ( zclWaterControl_TaskID, APP_SAVE_ATTRS_EVT, APP_SAVE_ATTRS_DELAY );
}

uint16 zclWaterControl_event_loop( uint8 task_id, uint16 events ) {
  if (events & SYS_EVENT_MSG) {
    afIncomingMSGPacket_t *MSGpkt;
    while ((MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(zclWaterControl_TaskID))) {
      LREP("MSGpkt->hdr.event 0x%X clusterId=0x%X\r\n", MSGpkt->hdr.event, MSGpkt->clusterId);
      switch (MSGpkt->hdr.event) {
      case KEY_CHANGE:
        zclWaterControl_HandleKeys(((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys);
        break;
        
      case ZCL_INCOMING_MSG:
        if (((zclIncomingMsg_t *)MSGpkt)->attrCmd) {
          osal_mem_free(((zclIncomingMsg_t *)MSGpkt)->attrCmd);
        }
        break;
        
      default:
        break;
      }
      osal_msg_deallocate((uint8 *)MSGpkt);
    }
    
    return (events ^ SYS_EVENT_MSG);
  }
  
  if (events & APP_REPORT_EVT) {
    zclWaterControl_Report();
    return (events ^ APP_REPORT_EVT);
  }

  if (events & APP_SAVE_ATTRS_EVT) {
      zclWaterControl_SaveAttributes();
      return (events ^ APP_SAVE_ATTRS_EVT);
  }
  
  if (events & APP_PUSH_STATE_EVT) {
      LREPMaster("[event_loop]: received APP_PUSH_STATE_EVT\r\n");
      zclWaterControl_PushState ();
      return (events ^ APP_PUSH_STATE_EVT);
  }
  
  return 0;
}

static void zclWaterControl_HandleKeys(byte portAndAction, byte keyCode) {

  LREP ( "[HandleKeys] portAndAction=0x%X keyCode=0x%X\r\n", portAndAction, keyCode );
  //HalLedSet(HAL_LED_1, HAL_LED_MODE_BLINK);
  
  //    HalLedSet(HAL_LED_2, HAL_LED_MODE_BLINK);
  //    HalLedSet(HAL_LED_3, HAL_LED_MODE_BLINK);
  //    zclFactoryResetter_HandleKeys(portAndAction, keyCode);
  // zclCommissioning_HandleKeys(portAndAction, keyCode);

  // if (portAndAction & HAL_KEY_PRESS) {
  //   LREPMaster("Key press\r\n");
  //   zclWaterControl_Report();
  // }
}

static void zclHot_BasicResetCB ( void ) { 
  uint8 index = zclWaterControl_GetEndpointIndex ( zclHotEndpoint.EndPoint );
  zclWaterControl_BasicResetCB ( &zcl_Configs[index] ); 
}

static void zclCold_BasicResetCB ( void ) {
  uint8 index = zclWaterControl_GetEndpointIndex ( zclColdEndpoint.EndPoint );
  zclWaterControl_BasicResetCB ( &zcl_Configs[index] ); 
}

static void zclWaterControl_BasicResetCB ( app_config_t *config ) {
  uint8 endpoint = config->Endpoint;

  LREP( "[ep%d BasicResetCB]\r\n", endpoint );

  zclWaterControl_ResetAttributesToDefaultValues ( config );

  config->Changed = TRUE;

  osal_set_event (zclWaterControl_TaskID, APP_SAVE_ATTRS_EVT );
}

static void zclHot_OnOffCB ( uint8 cmd ) {
  uint8 index = zclWaterControl_GetEndpointIndex ( zclHotEndpoint.EndPoint );
  
  zclWaterControl_OnOffCB ( &zcl_Configs[index], cmd ); 
}

static void zclCold_OnOffCB ( uint8 cmd ) {
  uint8 index = zclWaterControl_GetEndpointIndex ( zclColdEndpoint.EndPoint );
  
  zclWaterControl_OnOffCB ( &zcl_Configs[index], cmd ); 
}

static void zclWaterControl_OnOffCB ( app_config_t *config, uint8 cmd ) {
  uint8 endpoint = config->Endpoint; 

  LREP( "[ep%d OnOffCB] command: %d \r\n", endpoint, cmd );

  if (cmd == COMMAND_ON && config->Config.RelayState != 1 ) {
    config->Config.RelayState = 1;
    config->Changed = TRUE;
  } else if (cmd == COMMAND_OFF && config->Config.RelayState != 0 ) {
    config->Config.RelayState = 0;
    config->Changed = TRUE;
  } else if (cmd == COMMAND_TOGGLE) {
    config->Config.RelayState = !config->Config.RelayState;
    config->Changed = TRUE;
  } else {
    return;
  }

  osal_set_event (zclWaterControl_TaskID, APP_SAVE_ATTRS_EVT);

  zclWaterControl_ApplyRelay ( config );
  
  bdb_RepChangedAttrValue (endpoint, CID_ON_OFF, ATTRID_ON_OFF );
}

static void zclWaterControl_ApplyRelay ( app_config_t *config ) {
  //TODO: Impl it!!!
}

static void zclWaterControl_Increment ( app_config_t *config ) {
  uint48 *number = &(config->Config.CurrentSummDelivered);
    
  uint8 res;
  uint32 tmp;

  int i;
  for (i = 0; i < 6; ++i) {
    tmp = number->data[i];
    res = tmp + 1;
    number->data[i] = res;

    if (res > tmp) {
      break;
    }
  }
  
  config->Changed = TRUE;
  config->ReportCurrentSummDelivered = TRUE;
}

static void zclWaterControl_WriteAttrDataCB ( uint8 endpoint, zclAttrRec_t *pAttr ) {
  uint8 index = zclWaterControl_GetEndpointIndex (endpoint);
  zcl_Configs[index].Changed = TRUE;

  LREP ( "[ep%d WriteAttrDataCB] clusterId: %d, attrId: %d changed \r\n", endpoint,pAttr->clusterID, pAttr->attr.attrId );

  osal_set_event (zclWaterControl_TaskID, APP_SAVE_ATTRS_EVT );
}

static void zclWaterControl_SaveAttributes ( void ) {

  uint8 i = 0;  
  for (i = 0; i < zcl_Configs_AttrsCount; ++i) {
    if (zcl_Configs[i].Changed == FALSE) {
      continue;
    }

    uint8 endpoint = zcl_Configs[i].Endpoint; 
    endpoint_config_t *config  = &zcl_Configs[i].Config;
    uint16 nvId = zcl_Configs[i].NVkey;

    uint8 writeStatus = osal_nv_write ( nvId, 0, sizeof(endpoint_config_t), config );
  
    LREP ( "[ep%d SaveAttributes] zclHot_Config status: %d \r\n", endpoint, writeStatus );

    if ( writeStatus == SUCCESS ) {
        zcl_Configs[i].Changed = FALSE;
    }
  }
}

static void zclWaterControl_RestoreAttributes ( void ) {
  uint8 i = 0;  
  for (i = 0; i < zcl_Configs_AttrsCount; ++i) {
    uint8 endpoint = zcl_Configs[i].Endpoint; 
    endpoint_config_t *config = &zcl_Configs[i].Config;
    uint16 nvId = zcl_Configs[i].NVkey;

    uint8 status  = osal_nv_item_init ( nvId , sizeof(endpoint_config_t), NULL );
    LREP ( "[ep%d RestoreAttributes]: status=%d \r\n", endpoint, status );

    if ( status == NV_ITEM_UNINIT ) {
       uint8 writeStatus = osal_nv_write ( nvId, 0, sizeof(endpoint_config_t), config);
       LREP ( "[ep%d RestoreAttributes]: NV was empty, writing %d\r\n", endpoint, writeStatus );
    }

    if ( status == ZSUCCESS ) {
       LREP ( "[ep%d RestoreAttributes]: Reading from NV\r\n", endpoint );
       osal_nv_read ( nvId, 0, sizeof(endpoint_config_t), config );
    }
  }  
}

static void zclWaterControl_Report ( void ) {
  uint8 i = 0;
  uint8 changed = 0;

  for (i = 0; i < zcl_Configs_AttrsCount; ++i) {
    if (zcl_Configs[i].ReportCurrentSummDelivered == FALSE) { continue; }
    changed = changed + 1;
  }

  if ( changed == 0) { return; }

  zclReportCmd_t *pReportCmd;
  
  pReportCmd = osal_mem_alloc(sizeof(zclReportCmd_t) + (sizeof(zclReport_t)));

  if (pReportCmd != NULL) {
    pReportCmd->numAttr = 1;

    for (i = 0; i < zcl_Configs_AttrsCount; ++i) {
      if (zcl_Configs[i].ReportCurrentSummDelivered == FALSE) { continue; }

      uint8 endpoint = zcl_Configs[i].Endpoint;

      pReportCmd->attrList[0].attrID = ATTRID_CURRENT_SUMM_DELIVERED;
      pReportCmd->attrList[0].dataType = ZCL_UINT48;
      pReportCmd->attrList[0].attrData = (void *)(&zcl_Configs[i].Config.CurrentSummDelivered);

      zcl_SendReportCmd ( endpoint, &inderect_DstAddr, CID_METERING, pReportCmd, ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, bdb_getZCLFrameCounter() );

      zcl_Configs[i].ReportCurrentSummDelivered = FALSE;
      
      LREP ( "[ep%d Report] CurrentSummDelivered\r\n", endpoint );
    }
  }
  
  osal_mem_free(pReportCmd);
}

static void zclCommissioning_OnConnectCB ( void ) {
  osal_start_timerEx(zclWaterControl_TaskID, APP_PUSH_STATE_EVT, 20 * 1000);
}

static void zclWaterControl_PushState ( void ) {
  uint8 i = 0;

  for (i = 0; i < zcl_Configs_AttrsCount; ++i) {
    zcl_Configs[i].ReportCurrentSummDelivered = TRUE;
    bdb_RepChangedAttrValue (zcl_Configs[i].Endpoint, CID_ON_OFF, ATTRID_ON_OFF );
  }

  //send currentSummDelivered init values
  zclWaterControl_Report ();
}