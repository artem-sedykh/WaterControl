#include "zcl_watercontrol_data.h"
#include "zcl_watercontrol.h"
#include "zcl_general.h"

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

const uint8  zcl_Configs_AttrsCount              = ENDPOINTS_COUNT;

app_config_t zcl_Configs[ENDPOINTS_COUNT] = {
  { { {0, 0, 0, 0, 0, 0}, DEFAULT_DIVISOR, DEFAULT_MULTIPLIER, DEFAULT_STATUS, DEFAULT_SUMM_FORMATTING, DEFAULT_RELAY_STATE }, FALSE, NW_HOT_CONFIG,  1, TRUE },
  { { {0, 0, 0, 0, 0, 0}, DEFAULT_DIVISOR, DEFAULT_MULTIPLIER, DEFAULT_STATUS, DEFAULT_SUMM_FORMATTING, DEFAULT_RELAY_STATE }, FALSE, NW_COLD_CONFIG, 2, TRUE }
};

uint8 zclWaterControl_GetEndpointIndex ( uint8 endpoint ) {
  return endpoint - 1;
}

void zclWaterControl_ResetAttributesToDefaultValues ( app_config_t *config ) {
  config->Config.Divisor        = DEFAULT_DIVISOR;
  config->Config.Multiplier     = DEFAULT_MULTIPLIER;
  config->Config.Status         = DEFAULT_STATUS;
  config->Config.SummFormatting = DEFAULT_SUMM_FORMATTING;
  config->Config.RelayState     = DEFAULT_RELAY_STATE;
  
  uint8 i;
  for (i = 0; i < 6; ++i) {
    config->Config.CurrentSummDelivered.data[i] = 0;
  }
}
