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

//Preferences:
//=============
//set Standart preferences: {Sensor Interval: 0, min Soil-Humidity per pot: 1-9, min air temp: 10, min air humidity: 11}
static int preferences[12] = {30,0,0,0,0,0,0,0,0,0,40,10};

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
#define BTN_ENTER 3
volatile uint8_t nextMenuBtnToHandle = NULL_BUTTON_VALUE;  // for interrupt logic

// Sensors:
//=========
unsigned long SENSOR_READ_INTERVAL;  // long value in millis
// TODO!! :
#define APPROX_MAIN_LOOP_TIME 2000  // long value in millis

// SD-Card-Logger:
//default pin on mega are: 50 (MISO), 51 (MOSI), 52 (SCK), 53 (CS/SS).

// Soil Humidity sensors:
SoilWatering soilWatering;      // Declare general instance to use.
#define soilNodesRngStart 0     // TODO!! : Make sure these are correct!
#define needsWateringBelow 511  // TODO : test needed value + do we want individual setting section?
#define wateringInterval 3 // Time in ?unit
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
/*
  while (!bme.begin()) {
    logger.c("Could not find BME280 sensor!");
    delay(1000);
  }
*/
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
  
  //SD Card setup
  if (!SD.begin()) {
    logger.c("SD initialization failed");
  } else {
    logger.d("SD Card succesfully initialised");
  }
  //try to read preferences from SD Card, they remain standart if no SD-Card is found.
  readPreferences();
  uiMenu.begin(logger, getPreferences(), 14, printLcdText, storePreferences);
  //Read preferences are given to relevant variables
  long int SENSOR_READ_INTERVAL = (preferences[0]) * 1000;
  logger.d(SENSOR_READ_INTERVAL);

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
        logger.i("Down BTN");
        uiMenu.handleButtonDown();
        break;
      case BTN_UP:
      logger.i("Up BTN");
        uiMenu.handleButtonUp();
        break;
      case BTN_ENTER:
        logger.i("Enter BTN");
        uiMenu.handleButtonEnter();
        break;
    }
  }

  // SensorHandling:
  //================
  unsigned long currentMillis = millis();
  // TODO!: verify timing in regards to logic and program flow:
  if (currentMillis - previousSensorsMillis >= SENSOR_READ_INTERVAL) {
    logger.d("time to log! :" + (currentMillis));
    previousSensorsMillis = currentMillis;  // Saves current value, so that the time this section ran last, can be checked.

    //get values
    uint8_t* humidityData = soilWatering.collectSoilHumidityValues();
    float* bme280Data = getBME280Data(&logger);

    //store everything on SD-Card
    storeData(humidityData,bme280Data);

    soilWatering.toggleWatering();

  }
  
  if (currentMillis - previousWateringMillis >= wateringInterval + APPROX_MAIN_LOOP_TIME) {
    soilWatering.toggleWatering();
    previousWateringMillis = currentMillis;
  }
}
