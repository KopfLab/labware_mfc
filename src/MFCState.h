#pragma once
#include "device/SerialDeviceState.h"

// additional commands
#define CMD_UNIT_ID          "unit" // change unit ID
#define CMD_GAS              "gas" // change gas
#define CMD_TOTALIZER        "totalizer" // whether it has totalizer
  #define CMD_TOTALIZER_YES  "yes" // has totalizer
  #define CMD_TOTALIZER_NO   "no" // does not have totalizer

// M800 state
struct MFCState : public SerialDeviceState {

  char unit_id[2];
  char gas[10];
  bool has_totalizer;

  char P_units[10]; // pressure units
  char MF_units[10]; // mass flow units
  char VF_units[10]; // volumetric flow units
  char T_units[10]; // totalizer units

  MFCState() {};

  MFCState (bool locked, bool state_logging, bool data_logging, unsigned int data_reading_period_min, unsigned int data_reading_period, unsigned int data_logging_period, int data_logging_type, char the_unit_id, const char* the_gas, bool has_totalizer, const char* p_units, const char* mf_units, const char* vf_units, const char* t_units) :
    SerialDeviceState(locked, state_logging, data_logging, data_reading_period_min, data_reading_period, data_logging_period, data_logging_type), has_totalizer(has_totalizer) {
        unit_id[0] = the_unit_id;
        unit_id[1] = 0;
        strncpy(gas, the_gas, sizeof(gas) - 1); gas[sizeof(gas) - 1] = 0;
        strncpy(P_units, p_units, sizeof(P_units) - 1); P_units[sizeof(P_units) - 1] = 0;
        strncpy(MF_units, mf_units, sizeof(MF_units) - 1); MF_units[sizeof(MF_units) - 1] = 0;
        strncpy(VF_units, vf_units, sizeof(VF_units) - 1); VF_units[sizeof(VF_units) - 1] = 0;
        strncpy(T_units, t_units, sizeof(T_units) - 1); T_units[sizeof(T_units) - 1] = 0;
    };

  MFCState (bool locked, bool state_logging, bool data_logging, unsigned int data_reading_period_min, unsigned int data_reading_period, unsigned int data_logging_period, int data_logging_type, char the_unit_id, const char* the_gas, bool has_totalizer, const char* p_units, const char* mf_units, const char* vf_units) :
      MFCState(locked, state_logging, data_logging, data_reading_period_min, data_reading_period, data_logging_period, data_logging_type, the_unit_id, the_gas, has_totalizer, p_units, mf_units, vf_units, "") {}

};

/**** textual translations of state values ****/

// Unit ID
static void getStateUnitIDText(char* unit_id, char* target, int size, const char* pattern, bool include_key = true) {
  getStateStringText(CMD_UNIT_ID, unit_id, target, size, pattern, include_key);
}

static void getStateUnitIDText(char* unit_id, char* target, int size, bool value_only = false) {
  (value_only) ?
    getStateUnitIDText(unit_id, target, size, PATTERN_V_SIMPLE, false) :
    getStateUnitIDText(unit_id, target, size, PATTERN_KV_JSON_QUOTED, true);
}

// Gas
static void getStateGasText(char* gas, char* target, int size, const char* pattern, bool include_key = true) {
  getStateStringText(CMD_GAS, gas, target, size, pattern, include_key);
}

static void getStateGasText(char* gas, char* target, int size, bool value_only = false) {
  (value_only) ?
    getStateGasText(gas, target, size, PATTERN_V_SIMPLE, false) :
    getStateGasText(gas, target, size, PATTERN_KV_JSON_QUOTED, true);
}

// Totalizer
static void getStateTotalizerText(bool has_totalizer, char* target, int size, const char* pattern, bool include_key = true) {
  getStateBooleanText(CMD_TOTALIZER, has_totalizer, CMD_TOTALIZER_YES, CMD_TOTALIZER_NO, target, size, pattern, include_key);
}
static void getStateTotalizerText(bool has_totalizer, char* target, int size, bool value_only = false) {
  if (value_only) getStateTotalizerText(has_totalizer, target, size, PATTERN_V_SIMPLE, false);
  else getStateTotalizerText(has_totalizer, target, size, PATTERN_KV_JSON_QUOTED, true);
}
