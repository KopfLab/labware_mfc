#pragma SPARK_NO_PREPROCESSOR // disable spark preprocssor to avoid issues with callbacks
#include "application.h"

// debugging options
//#define CLOUD_DEBUG_ON
//#define WEBHOOKS_DEBUG_ON
//#define STATE_DEBUG_ON
//#define DATA_DEBUG_ON
//#define SERIAL_DEBUG_ON
//#define LCD_DEBUG_ON

// keep track of installed version
#define STATE_VERSION    3 // update whenver structure changes
#define DEVICE_VERSION  "mfc 0.3.2" // update with every code update

// scale controller
#include "MFCController.h"

// lcd
DeviceDisplay* lcd = &LCD_20x4;

// initial state of the scale
MFCState* state = new MFCState(
  /* locked */                    false,
  /* state_logging */             false,
  /* data_logging */              false,
  /* data_reading_period_min */   2000, // in ms
  /* data_reading_period */       5000, // in ms
  /* data_logging_period */       600, // in seconds
  /* data_logging_type */         LOG_BY_TIME,
  /* unit_id */                   'A',
  /* gas */                       "N2",
  /* has totalizer */             true,
  /* P units */                   "bar",
  /* mass flow units */           "Sml/min",
  /* volume flow units */         "ml/min",
  /* totalizer units */           "SL"
);

// scale controller
MFCController* mfc = new MFCController(
  /* reset pin */         A5,
  /* lcd screen */        lcd,
  /* baud rate */         19200,
  /* serial config */     SERIAL_8N1,
  /* error wait */        500,
  /* pointer to state */  state
);

// using system threading to speed up restart after power out
SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

// setup
void setup() {

  // serial
  Serial.begin(9600);

  // lcd temporary messages
  lcd->setTempTextShowTime(3); // how many seconds temp time

  // mfc
  mfc->init();

  // connect device to cloud
  Serial.println("INFO: connecting to cloud");
  Particle.connect();
}

// loop
void loop() {
  mfc->update();
}
