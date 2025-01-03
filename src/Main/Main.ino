#include <LiquidCrystal.h>
#include <BME280I2C.h>     // https://github.com/finitespace/BME280
#include <Wire.h>          // (I2C) for BME280
#include <SPI.h>           // Used for SD-Card
#include <SD.h>            // https://github.com/arduino-libraries/SD
#include <SimpleLogger.h>  // for debugging, https://github.com/DouglasFlores-fun/SimpleLogger
#include "SoilWatering.h"  // Custom library made by us, to have a cleaner code structure.
#include <sdCard.ino>

#define SERIAL_BAUD 115200  // Rate used for serial communication

// Logging (for debugging):
//========================
// Supported levels: CRITICAL WARNING INFO DEBUG via logger.c , .w or .[...]
SimpleLogger logger(LOG_LEVEL_DEBUG);

// MultiTasking:
//=============

unsigned long previousMillis = 0;  // will store last time main loop iterrated

// Main Menu:
//===========

// Define Pins used for LCD Display
LiquidCrystal lcd(4, 5, 14, 15, 16, 17);
String line1;  //For more efficient way to handle prints.
String line2;

// Menu navigation buttons
#define NULL_BUTTON_VALUE 0  // Null value
#define BTN_DOWN 18
#define BTN_UP 19
#define BTN_ENTER 2
volatile uint8_t nextMenuBtnToHandle = NULL;  // for interrupt logic, TODO

// Special Button:
#define EMERGENCY_STOP_BUTTON 3

// Main menu page selctor:
bool notInSubMenu = true;
int menuPage = 0;
#define MENU_PAGE_COUNT 5
const char* mainPageNames[MENU_PAGE_COUNT] = { "First", "Seccond", "Third", "Fourth", "Fifth" };

// Sub menu implementation:
int subPageValue = 0;
int currSubPageCount = 1;                                                                                        // init with dummy value
const int subPageValuesRange[MENU_PAGE_COUNT] = { 3, 3, 3, 3, 5 };                                               // top range of parameterable sub page values
const int subPageNameAssignment[MENU_PAGE_COUNT]{ 0, 0, 0, 0, 1 };                                               // Which naming scheme is used for the subpage variables, selects index of "subPageStandardValueNames"
const char* subPageStandardValueNames[MENU_PAGE_COUNT][7] = { { "Yes", "No", "Cancel" }, { 0, 1, 2, -2, -1 } };  // TODO: check 2nd dimension array size

// Sensors:
//=========

#define SENSOR_READ_INTERVAL 30000  // long value in millis

// SD-Card-Logger:
//default pin on mega are: 50,51,52 and 53 is the SS pin

// Soil Humidity sensors:
SoilWatering soilWatering;      // Declare general instance to use.
#define soilNodesRngStart 0     // TODO!! : Make sure these are correct!
#define needsWateringBelow 511  // TODO : test needed value + do we want individual setting section?

// Temp/Humid/Pressure sensor:
BME280I2C bme;


void setup() {
  // DEBUGGING:
  //===========
  pinMode(LED_BUILTIN, OUTPUT);  //For debugging
  Serial.begin(SERIAL_BAUD);     //For debugging
  while (!Serial) {}             // Wait
  logger.enable(true);
  //logger.info("Test logger");
  //logger.i("Test same logger");

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
  // LCD:
  lcd.begin(16, 2);
  line1.reserve(17);
  line2.reserve(17);
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
    logger.d("SD Card succesfully initialised")
  }
  //try to read preferences from SD Card
  sdCard.readPreferences(&logger);
}

void loop() {
  unsigned long currentMillis = millis();

  // MainMenu:
  //==========
  // Menu Buttons

  //No interrupt safety needed, since atomic operations (and overwriting value to null even if interrupt occures inbetween ok, since the other input not yet handled):
  uint8_t btnCurrentlyHandled = nextMenuBtnToHandle;
  nextMenuBtnToHandle = NULL_BUTTON_VALUE;

  if (btnCurrentlyHandled != NULL_BUTTON_VALUE) {
    switch (btnCurrentlyHandled) {
      case BTN_DOWN:
        if (notInSubMenu) {
          menuPage = unsignedModulo(menuPage - 1, MENU_PAGE_COUNT);
          printLcdText("On Page:" + String(menuPage), "");
        } else {
          subPageValue = unsignedModulo(subPageValue - 1, currSubPageCount);
          printLcdText("Page:" + String(menuPage), "On Sub Value:" + String(subPageValue));
        }
        delay(500);
        break;
      case BTN_UP:
        if (notInSubMenu) {
          menuPage = unsignedModulo(menuPage + 1, MENU_PAGE_COUNT);
          printLcdText("On Page:" + menuPage, "");
        } else {
          subPageValue = unsignedModulo(subPageValue + 1, currSubPageCount);

          printLcdText("Page:", menuPage, "On Sub Value:", subPageValue);
        }
        delay(500);
        break;
      case BTN_ENTER:
        if (notInSubMenu) {
          printLcdText("Entered:" + menuPage, "");
          currSubPageCount = subPageValuesRange[menuPage];  // Set the range of possible values for this sub page
          notInSubMenu = false;
        } else {
          printLcdText("Exited:" + menuPage, "");
          notInSubMenu = true;
        }
        delay(500);
        break;
    }
  }

  // SensorHandling:
  //================
  // TODO!: verify timing in regards to logic and program flow:
  if (currentMillis - previousMillis >= SENSOR_READ_INTERVAL) {
    previousMillis = currentMillis;  // Saves current value, so that the time this section ran last, can be checked.

    //get values
    uint8_t* humidityData = soilWatering.collectSoilHumidityValues();
    float* bme280Data = storeBME280Data(&logger);

    //store everything on SD-Card
    sdCard.storeData(humidityData,bme280Data,&logger);

    //Check if plants need watering
    soilWatering.toggleWatering();
  }
}
