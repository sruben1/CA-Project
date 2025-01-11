#include <LiquidCrystal.h>
#include <BME280I2C.h>     // https://github.com/finitespace/BME280
#include <Wire.h>          // (I2C) for BME280
#include <SPI.h>           // Used for SD-Card
#include <SD.h>            // https://github.com/arduino-libraries/SD
#include <SimpleLogger.h>  // for debugging, https://github.com/DouglasFlores-fun/SimpleLogger
#include "SoilWatering.h"  // Custom library made by us, to have a cleaner code structure.
#include "UiMenu.h"  // Custom library made by us, to have a cleaner code structure.

#define SERIAL_BAUD 115200  // Rate used for serial communication

// Logging (for debugging):
//========================
// Supported levels: CRITICAL WARNING INFO DEBUG via logger.c , .w or .[...]
SimpleLogger logger(LOG_LEVEL_DEBUG);

// MultiTasking:
//=============

unsigned long previousSensorsMillis = 0;  // will store last time sensors iterrated
unsigned long previousWateringMillis = 0;  // will store last time watering run iterrated

// Main Menu:
//===========

// Define Pins used for LCD Display
LiquidCrystal lcd(4, 5, 14, 15, 16, 17);

//Menu:
UiMenu uiMenu;

// Menu navigation buttons
#define NULL_BUTTON_VALUE 0  // Null value
#define BTN_DOWN 18
#define BTN_UP 19
#define BTN_ENTER 2
volatile uint8_t nextMenuBtnToHandle = NULL;  // for interrupt logic, TODO

// Special Button:
#define EMERGENCY_STOP_BUTTON 3

// Sensors:
//=========

#define SENSOR_READ_INTERVAL 30000  // long value in millis
// TODO!! :
#define APPROX_MAIN_LOOP_TIME 2000  // long value in millis

// SD-Card-Logger:
//default pin on mega are: 50 (MISO), 51 (MOSI), 52 (SCK), 53 (CS/SS).

// Soil Humidity sensors:
SoilWatering soilWatering;      // Declare general instance to use.
#define soilNodesRngStart 0     // TODO!! : Make sure these are correct!
#define needsWateringBelow 511  // TODO : test needed value + do we want individual setting section?
unsigned int howLongToWater = 0; 

// Temp/Humid/Pressure sensor:
BME280I2C bme;


void setup() {
  // DEBUGGING:
  //===========
  pinMode(LED_BUILTIN, OUTPUT);  //For debugging
  Serial.begin(SERIAL_BAUD);     //For debugging
  while (!Serial) {}             // Wait
  logger.enable(true);
  logger.i("Booting up Arduino...");

  // SENSORS:
  //=========
  soilWatering.begin(soilNodesRngStart, needsWateringBelow, &logger);
  Wire.begin();

  while (!bme.begin()) {
    logger.c("Could not find BME280 sensor!");
    delay(1000);
  }

  switch (bme.chipModel()) {
    case BME280::ChipModel_BME280:
      logger.d("Found BME280 sensor! Success.");
      break;
    case BME280::ChipModel_BMP280:
      logger.d("Found BMP280 sensor! No Humidity available.");
      break;
    default:
      logger.c("Found UNKNOWN sensor! Error!");
  }

  // UI Menu:
  //=========
  uiMenu.begin(logger, getPreferences(), 14, printLcdText, storePreferences);
  // LCD:
  lcd.begin(16, 2);
  //line1.reserve(17);
  //line2.reserve(17);
  //Buttons:
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(BTN_DOWN), downButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_UP), upButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_ENTER), enterButtonInterrupt, RISING);

  // Emrgencey stop:
  pinMode(EMERGENCY_STOP_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EMERGENCY_STOP_BUTTON), emergencyStop, RISING);

  //SD Card setup
  if (!SD.begin()) {
    logger.c("SD initialization failed");
  } else {
    logger.d("SD Card succesfully initialised");
  }
  //try to read preferences from SD Card
  readPreferences(&logger);

  logger.d("Setup finished!");
}

void loop() {
  // MainMenu:
  //==========
  // Menu Buttons

  //No interrupt safety needed, since atomic operations (and overwriting value to null even if interrupt occures inbetween ok, since the other input not yet handled):
  uint8_t btnCurrentlyHandled = nextMenuBtnToHandle;
  nextMenuBtnToHandle = NULL_BUTTON_VALUE;

  if (btnCurrentlyHandled != NULL_BUTTON_VALUE) {
    switch (btnCurrentlyHandled) {
      case BTN_DOWN:
        uiMenu.handleButtonDown();
        break;
      case BTN_UP:
        uiMenu.handleButtonUp();
        break;
      case BTN_ENTER:
        uiMenu.handleButtonEnter();
        break;
    }
  }

  // SensorHandling:
  //================
  unsigned long currentMillis = millis();
  // TODO!: verify timing in regards to logic and program flow:
  if (currentMillis - previousSensorsMillis >= SENSOR_READ_INTERVAL) {
    previousSensorsMillis = currentMillis;  // Saves current value, so that the time this section ran last, can be checked.

    //get values
    uint8_t* humidityData = soilWatering.collectSoilHumidityValues();
    float* bme280Data = getBME280Data(&logger);

    //store everything on SD-Card
    storeData(humidityData,bme280Data,&logger);


  }
  /*
  // Calculate the available time for watering
  unsigned long availableTime = SENSOR_READ_INTERVAL - (howLongToWater + APPROX_MAIN_LOOP_TIME);

  // Ensure available time is positive
  if (availableTime <= 0) {
      availableTime = howLongToWater; // Minimum available time must fit the watering task
  }

  // Dynamically calculate the fraction to balance the interval
  float dynamicFraction = (float) howLongToWater / availableTime;

  // Clamp the fraction to a maximum of 1.0
  if (dynamicFraction > 1.0) {
      dynamicFraction = 1.0;
  }

  // Adjust the watering interval using the dynamic fraction
  unsigned long adjustedWateringInterval = availableTime * dynamicFraction;

  // Perform the conditional check
  if (currentMillis - previousWateringMillis >= adjustedWateringInterval + APPROX_MAIN_LOOP_TIME) {
          //Check if plants need watering
    soilWatering.toggleWatering();
    previousWateringMillis = currentMillis;
  }
  */
}
