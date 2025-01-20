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
//set standart preferences: {Sensor Interval: 30, min Soil-Humidity per pot (1 to 9): 0-11, min air temp: 40, min air humidity: 10}
static int preferences[12] = {30,0,0,0,0,0,0,0,0,0,40,10};
//Insert all the water values here as well
int maxAirTemperature;
int maxAirHumidity;

// MultiTasking:
//=============
unsigned long previousSensorsMillis = 0;  // will store last time sensors iterrated
unsigned long previousWateringMillis = 0;  // will store last time watering run iterrated
static bool shutDownNextIteration = false;

// Main Menu:
//===========

// Define Pins used for LCD Display
LiquidCrystal lcd(6, 7, 14, 15, 16, 17);

//User interface handler class:
UiMenu uiMenu;

//Status LEDs:
#define LED_GREEN 4
#define LED_RED 5

// Menu navigation buttons
#define NULL_BUTTON_VALUE 0  // Null value
#define BTN_DOWN 18
#define BTN_UP 19
#define BTN_ENTER 3
volatile uint8_t nextMenuBtnToHandle = NULL_BUTTON_VALUE;  // for interrupt logic

// Sensors:
//=========
unsigned long SENSOR_READ_INTERVAL;  // long value in millis

// SD-Card-Logger:
//default pin on mega are: 50 (MISO), 51 (MOSI), 52 (SCK), 53 (CS/SS).

// Soil Humidity sensors:
SoilWatering soilWatering;      // Declare general instance to use.
#define soilNodesRngStart 0     

// Soil watring timing:
#define howLongToWater 3000 // Time in millis
#define waterAPlantEvery 15000 // Time in millis

// Servo valve:
#define SRVO_VALVE_PWM 8 

// Temp/Humid/Pressure sensor:
//default I2C pins on mega are: 20 (SDA) and 21 (SCL)
BME280I2C bme;


// Ventilation:
//============
#define FAN 10

// Stepper motors:
//================ 
// Pins: TODO: Replace with correct pins
#define motorPinX1 22
#define motorPinX2 24
#define motorPinX3 26
#define motorPinX4 28

#define motorPinY1 23
#define motorPinY2 25
#define motorPinY3 27
#define motorPinY4 29

// Home switches
#define HOME_SWITCH_PIN_X 11
#define HOME_SWITCH_PIN_Y 12

void setup() {
  // DEBUGGING:
  //===========
  pinMode(LED_BUILTIN, OUTPUT);  //For debugging
  Serial.begin(SERIAL_BAUD);     //For debugging

  #if true // Set to true if logging should be on.
    while (!Serial) {}             // Wait
    logger.enable(true);
  #else 
    logger.enable(false);
  #endif

  logger.i("Booting up Arduino...");

  // SENSORS:
  //=========
  Wire.begin();
  int attempts = 0;
  while (!bme.begin()) {
    logger.c("Could not find BME280 sensor!");
    delay(1000);
    //If Sensor can't be found within 4 iterations, stop looking and continue
    if(attempts > 3){
      break;
    }
    attempts++;
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
  // LCD:
  lcd.begin(16, 2);
 

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
  // Try to read preferences from SD Card, they remain standart if no SD-Card is found.
  readPreferences();

  // Distribute the Preferences loaded form the File (or default):
  SENSOR_READ_INTERVAL = (preferences[0]) * 1000;
  maxAirTemperature = preferences[10];
  maxAirHumidity = preferences[11];
  // Copy lower soil watering threasholds (and scale value to full range):
  static int frozenSoilThreasholdPreferences[9];
  for(int i = 0; i < 9; i++) { 
    frozenSoilThreasholdPreferences[i] = preferences[i + 1] * 93;
  }

  //Initialize Control Pin for Fan
  pinMode(FAN, OUTPUT);
  //Intialize Control Pin for Limit switches
  pinMode(HOME_SWITCH_PIN_X, INPUT);
  pinMode(HOME_SWITCH_PIN_Y, INPUT);
  // Initialize alle values that are important to the watering system. DO NOT CHANGE THE ORDER OF PINS for the steppers
  soilWatering.begin(soilNodesRngStart, frozenSoilThreasholdPreferences, howLongToWater, logExportFunction, motorPinX1, motorPinX3, motorPinX2, motorPinX4, motorPinY1, motorPinY3, motorPinY2, motorPinY4, SRVO_VALVE_PWM, HOME_SWITCH_PIN_X, HOME_SWITCH_PIN_Y);
  uiMenu.begin(logger, getPreferences(), 14, LED_GREEN, LED_RED, printLcdText, storePreferences, &shutDownNextIteration);
  soilWatering.homeStepper();
  logLongUnsigned("Sensor read interval := ", SENSOR_READ_INTERVAL); // (Debug)

  logger.d("Setup finished!");
}

void loop() {
  // Implementation to safely shut down system:
  if (shutDownNextIteration){
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
    printLcdText("Now safe to"," shut down.");
    while(true){
      //wait for user to reset or disconnect power.
    }
  }
  // UI Menu:
  //===========
  // Menu Buttons:
  // Here no interrupt safety needed, since atomic operations (and overwriting value to null even
  // if interrupt occures inbetween ok, since the other input not yet handled):
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

  // Sensor Handling:
  //=================
  unsigned long currentMillis = millis();
  /*char buffer[64];
  sprintf(buffer, "Sens bool: %lu ; delta: %lu", (currentMillis - previousSensorsMillis) > SENSOR_READ_INTERVAL, currentMillis - previousSensorsMillis);
  logger.d(buffer);*/ // Used to debug timing
  if ((currentMillis - previousSensorsMillis) >= SENSOR_READ_INTERVAL /3) {

    logLongUnsigned("time to log!: ", currentMillis);
    //logLongUnsigned("Sensor read interval is: ", SENSOR_READ_INTERVAL);
    
    //get values
    uint8_t* humidityData = soilWatering.collectSoilHumidityValues();
    float* bme280Data = getBME280Data(&logger);
    ventilationCheck(bme280Data);
    //store everything on SD-Card
    storeData(humidityData,bme280Data);
    
    previousSensorsMillis = currentMillis;  // Saves current value, so that the time this section ran last, can be checked.
  }

  if ((currentMillis - previousWateringMillis) >= waterAPlantEvery) {
  logLongUnsigned("time to water! :", currentMillis);

  printBusy();
  soilWatering.toggleWatering();
  endBusy();
  
  previousWateringMillis = currentMillis; // Saves current value, so that the time this section ran last, can be checked.
  }
}
