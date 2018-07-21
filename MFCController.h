#pragma once
#include "MFCState.h"
#include "device/SerialDeviceController.h"

// serial communication constants
//#define MFC_DATA_REQUEST  "D00Z" // data request command
//#define MFC_DATA_N_MAX    12
// NOTE: consider making this dynamic but keep in mind that around 12 the 600 chars overflow for state info!
// FIXME: not unusal to have 12 variables from the MFC (4 devices x 3 vars, what to do in this case?)

// controll information
//#define RS485TxControl    D3


//#define MFC_DATA_DELIM 		   9  // value/variable pair delimiter (tab)
//#define MFC_DATA_END         13 // end of data stream (return character)

#define MFC_DATA_DELIM        32 // value delimiter (space)
#define MFC_DATA_END          13 // end of data stream (return character)

// controller class
class MFCController : public SerialDeviceController {

  private:

    // state
    MFCState* state;
    SerialDeviceState* dss = state;
    DeviceState* ds = state;

    // serial communication
    int val_expected;
    int val_counter;
    bool at_delimiter;

    // constructor
    void construct();
    void updateValExpected();
    void updateUnits();

  public:

    // constructors
    MFCController();

    // without LCD
    MFCController (int reset_pin, const long baud_rate, const long serial_config, const int error_wait, MFCState* state) :
      SerialDeviceController(reset_pin, baud_rate, serial_config, "", error_wait), state(state) { construct(); }

    // with LCD
    MFCController (int reset_pin, DeviceDisplay* lcd, const long baud_rate, const long serial_config, const int error_wait, MFCState* state) :
      SerialDeviceController(reset_pin, lcd, baud_rate, serial_config, "", error_wait), state(state) { construct(); }

    // setup & loop
    void init();

    // serial
    void sendRequestCommand();
    void startSerialData();
    int processSerialData(byte b);
    void completeSerialData(int error_count);

    // data
    void updateDataInformation();

    // state
    void assembleStateInformation();
    void assembleDisplayStateInformation();
    DeviceState* getDS() { return(ds); }; // return device state
    SerialDeviceState* getDSS() { return(dss); }; // return device state serial
    void saveDS(); // save device state to EEPROM
    bool restoreDS(); // load device state from EEPROM

    // particle commands
    void parseCommand (); // parse a cloud command
    bool changeUnitID(char unit_id);
    bool parseUnitID();
    bool changeGas(char* gas);
    bool parseGas();
    bool changeTotalizer(bool has_totalizer);
    bool parseTotalizer();
};

/**** CONSTRUCTION & init ****/

void MFCController::construct() {
  // start data vector
  data.resize(6);
  data[0] = DeviceData(1, "P");
  data[1] = DeviceData(2, "T", "degC"); // assume this is always in degC
  data[2] = DeviceData(5, "flow"); // volumetric
  data[3] = DeviceData(3, "flow"); // mass flow
  data[4] = DeviceData(4, "setpoint");
  data[5] = DeviceData(6, "total"); // only available in units with totalizer
}

void MFCController::updateValExpected() {
  val_expected = (state->has_totalizer) ? 6 : 5;
  resetData(); // reset all data
}

void MFCController::updateUnits() {
  char units[20];
  snprintf(units, sizeof(units), "%s %s", state->P_units, state->gas); data[0].setUnits(units);
  snprintf(units, sizeof(units), "%s %s", state->VF_units, state->gas); data[2].setUnits(units);
  snprintf(units, sizeof(units), "%s %s", state->MF_units, state->gas); data[3].setUnits(units); data[4].setUnits(units);
  snprintf(units, sizeof(units), "%s %s", state->T_units, state->gas); data[5].setUnits(units);
  resetData(); // reset all data
}

// init function
void MFCController::init() {
  SerialDeviceController::init();
  updateValExpected();
  updateUnits();
}

/**** SERIAL COMMUNICATION ****/

void MFCController::sendRequestCommand() {
  Serial1.print(state->unit_id[0]);
  Serial1.print("\r");
}

void MFCController::startSerialData() {
  SerialDeviceController::startSerialData();
  val_counter = 0;
  at_delimiter = false;
}

int MFCController::processSerialData(byte b) {
  // keep track of all data
  SerialDeviceController::processSerialData(b);

  // pattern interpretation
  char c = (char) b;

  if (c == 0) {
    // do nothing when 0 character encountered
  } else if (c == MFC_DATA_DELIM || c == MFC_DATA_END) {
    if (!at_delimiter) {
      // end of value

      if (val_counter == 0) {
        // should be the unit
        if (value_buffer[0] != state->unit_id[0]) {
          Serial.printf("WARNING: not the correct unit, expected '%s', found '%s'\n", state->unit_id, value_buffer);
          if (lcd) lcd->printLineTemp(1, "MFC: wrong unit");
          error_counter++;
        }
      } else if (val_counter <= val_expected) {
        // process value
        bool valid_value = data[val_counter - 1].setNewestValue(value_buffer, true, true, 1L);
        if (!valid_value) {
          Serial.printf("WARNING: problem %d with serial data for %s value: %s\n", error_counter, data[val_counter - 1].variable, value_buffer);
          snprintf(lcd_buffer, sizeof(lcd_buffer), "MFC: %d value error", error_counter);
          if (lcd) lcd->printLineTemp(1, lcd_buffer);
          error_counter++;
        }
      } else if (val_counter == val_expected + 1) {
        // should be gas
        if (strcmp(value_buffer, state->gas) != 0) {
          Serial.printf("WARNING: not the correct gas, expected '%s', found '%s'\n", state->gas, value_buffer);
          snprintf(lcd_buffer, sizeof(lcd_buffer), "MFC gas: %s!=%s", state->gas, value_buffer);
          if (lcd) lcd->printLineTemp(1, lcd_buffer);
          error_counter++;
        }
      } else {
        // additional info after the gas
      }

      // reset value
      resetSerialValueBuffer();
      val_counter++;
      at_delimiter = true;
    }
  } else if (c >= 32 && c <= 126) {
    // regular value
    at_delimiter = false;
    appendToSerialValueBuffer(c);
  } else {
    // unrecognized part of data
    return(SERIAL_DATA_ERROR);
  }

  // return complete once at end
  if (c == MFC_DATA_END) {
   // end of data transmission
   if (val_counter - 2 < val_expected) {
     Serial.printf("WARNING: failed to receive expected number (%d) of values, found only %d\n", val_expected, val_counter - 2);
     if (lcd) lcd->printLineTemp(1, "MFC: incomplete msg");
     error_counter++;
   }
   return(SERIAL_DATA_COMPLETE);
 }

  return(SERIAL_DATA_WAITING);
}

void MFCController::completeSerialData(int error_count) {
  // only save values if all of them where received properly
  if (error_count == 0) {
    #ifdef SERIAL_DEBUG_ON
      Serial.printf("INFO: saving %d values\n", val_expected);
    #endif
    for (int i=0; i < val_expected; i++) data[i].saveNewestValue(true); // average for all valid data
  }
}

/****** STATE INFORMATION *******/

void MFCController::assembleStateInformation() {
  SerialDeviceController::assembleStateInformation();
  char pair[60];
  getStateUnitIDText(state->unit_id, pair, sizeof(pair)); addToStateInformation(pair);
  getStateGasText(state->gas, pair, sizeof(pair)); addToStateInformation(pair);
  getStateTotalizerText(state->has_totalizer, pair, sizeof(pair)); addToStateInformation(pair);
}

void MFCController::assembleDisplayStateInformation() {
  SerialDeviceController::assembleDisplayStateInformation();
  // unit ID
  uint i = strlen(lcd_buffer);
  lcd_buffer[i++] = ' ';
  lcd_buffer[i++] = state->unit_id[0];
  lcd_buffer[i] = 0;
}


/***** DATA INFORMATION ****/

void MFCController::updateDataInformation() {
  SerialDeviceController::updateDataInformation();
  // LCD update
  if (lcd) {

    int i;

    // pressure
    i = 0;
    if (data[i].getN() > 0)
      getDataDoubleText(data[i].variable, data[i].getValue(), data[i].units, data[i].getN(), lcd_buffer, sizeof(lcd_buffer), PATTERN_KVUN_SIMPLE, data[i].getDecimals());
    else
      getInfoKeyValue(lcd_buffer, sizeof(lcd_buffer), data[i].variable, "no data yet", PATTERN_KV_SIMPLE);
    lcd->printLine(2, lcd_buffer);

    // temperature
    i = 1;
    if (data[i].getN() > 0)
      getDataDoubleText(data[i].variable, data[i].getValue(), data[i].units, data[i].getN(), lcd_buffer, sizeof(lcd_buffer), PATTERN_KVUN_SIMPLE, data[i].getDecimals());
    else
      getInfoKeyValue(lcd_buffer, sizeof(lcd_buffer), data[i].variable, "no data yet", PATTERN_KV_SIMPLE);
    lcd->printLine(3, lcd_buffer);

    // mass flow
    i = 3;
    if (data[i].getN() > 0)
      getDataDoubleText("F", data[i].getValue(), data[i].units, data[i].getN(), lcd_buffer, sizeof(lcd_buffer), PATTERN_KVUN_SIMPLE, data[i].getDecimals());
    else
      getInfoKeyValue(lcd_buffer, sizeof(lcd_buffer), "F", "no data yet", PATTERN_KV_SIMPLE);
    lcd->printLine(4, lcd_buffer);

  }
}

/**** STATE PERSISTENCE ****/

// save device state to EEPROM
void MFCController::saveDS() {
  EEPROM.put(STATE_ADDRESS, *state);
  #ifdef STATE_DEBUG_ON
    Serial.println("INFO: MFC state saved in memory (if any updates were necessary)");
  #endif
}

// load device state from EEPROM
bool MFCController::restoreDS(){
  MFCState saved_state;
  EEPROM.get(STATE_ADDRESS, saved_state);
  bool recoverable = saved_state.version == STATE_VERSION;
  if(recoverable) {
    EEPROM.get(STATE_ADDRESS, *state);
    Serial.printf("INFO: successfully restored state from memory (version %d)\n", STATE_VERSION);
  } else {
    Serial.printf("INFO: could not restore state from memory (found version %d), sticking with initial default\n", saved_state.version);
    saveDS();
  }
  return(recoverable);
}


/****** STATE CHANGES & WEB COMMAND PROCESSING *******/

void MFCController::parseCommand() {

  SerialDeviceController::parseCommand();

  if (command.isTypeDefined()) {
    // command processed successfully by parent function
  } else if (parseUnitID()) {
    // unit ID
  } else if (parseGas()) {
    // gas
  } else if (parseTotalizer()) {
    // totalizer
  }


  // more additional, device specific commands

}

bool MFCController::changeUnitID (char unit_id) {
  bool changed = unit_id != state->unit_id[0];

  if (changed) state->unit_id[0] = unit_id;

  #ifdef STATE_DEBUG_ON
    (changed) ?
      Serial.printf("INFO: changing unit ID to %s\n", state->unit_id) :
      Serial.printf("INFO: unit ID is already %s\n", state->unit_id);
  #endif

  if (changed) saveDS();

  return(changed);
}

bool MFCController::parseUnitID() {
  if (command.parseVariable(CMD_UNIT_ID)) {
    command.extractValue();
    command.success(changeUnitID(command.value[0]));
    getStateUnitIDText(state->unit_id, command.data, sizeof(command.data));
  }
  return(command.isTypeDefined());
}

bool MFCController::changeGas (char* gas) {
  bool changed = strcmp(gas, state->gas) != 0;

  if (changed) {
    strncpy(state->gas, gas, sizeof(state->gas) - 1);
    state->gas[sizeof(state->gas) - 1] = 0; // just for safety
    updateUnits();
  }

  #ifdef STATE_DEBUG_ON
    (changed) ?
      Serial.printf("INFO: changing gas to '%s'\n", state->gas) :
      Serial.printf("INFO: gas is already '%s'\n", state->gas);
  #endif

  if (changed) saveDS();

  return(changed);
}

bool MFCController::parseGas() {
  if (command.parseVariable(CMD_GAS)) {
    command.extractValue();
    command.success(changeGas(command.value));
    getStateGasText(state->gas, command.data, sizeof(command.data));
  }
  return(command.isTypeDefined());
}

bool MFCController::changeTotalizer (bool has_totalizer) {

  bool changed = has_totalizer != state->has_totalizer;

  if (changed) {
    state->has_totalizer = has_totalizer;
    updateValExpected();
  }

  #ifdef STATE_DEBUG_ON
    if (changed)
      on ? Serial.println("INFO: yes totalizer") : Serial.println("INFO: no totalizer");
    else
      on ? Serial.println("INFO: totalizer already yes") : Serial.println("INFO: totalizer already no");
  #endif

  if (changed) saveDS();

  return(changed);
}

bool MFCController::parseTotalizer() {
  if (command.parseVariable(CMD_TOTALIZER)) {
    // parse calc rate
    command.extractValue();
    if (command.parseValue(CMD_TOTALIZER_YES)) {
      command.success(changeTotalizer(true));
    } else if (command.parseValue(CMD_TOTALIZER_NO)) {
      command.success(changeTotalizer(false));
    }
    getStateTotalizerText(state->has_totalizer, command.data, sizeof(command.data));
  }
  return(command.isTypeDefined());
}
