#include <LiquidCrystal.h>
#include <BME280I2C.h>     // https://github.com/finitespace/BME280
#include <Wire.h>          // (I2C)
#include <SPI.h>           // Used for SD-Card
#include <SD.h>            //https://github.com/arduino-libraries/SD
#include "SoilWatering.h"  //Custom library made by us, to have a cleaner code structure.

#define SERIAL_BAUD 115200  // Serial frequency

//MultiTasking:
//============

unsigned long previousMillis = 0;  // will store last time main loop recurred

//Main Menu:
//============

// Define Pins used for LCD Display
LiquidCrystal lcd(12, 11, 9, 8, 7, 6);
String line1;  //For more efficient way to handle prints.
String line2;

//Menu navigation buttons
#define BTN_DOWN 2
#define BTN_UP 3
#define BTN_ENTER 4
// => Range of these buttons:
#define BTN_RNG_START 2
#define BTN_RNG_END 4

//Main menu page selctor:
bool notInSubMenu = true;
int menuPage = 0;
#define MENU_PAGE_COUNT 5
const char* mainPageNames[MENU_PAGE_COUNT] = { "First", "Seccond", "Third", "Fourth", "Fifth" };

//Sub menu implementation:
int subPageValue = 0;
int currSubPageCount = 1;                                                                                        //init with dummy value
const int subPageValuesRange[MENU_PAGE_COUNT] = { 3, 3, 3, 3, 5 };                                               // top range of parameterable sub page values
const int subPageNameAssignment[MENU_PAGE_COUNT]{ 0, 0, 0, 0, 1 };                                               //Which naming scheme is used for the subpage variables, selects index of "subPageStandardValueNames"
const char* subPageStandardValueNames[MENU_PAGE_COUNT][7] = { { "Yes", "No", "Cancel" }, { 0, 1, 2, -2, -1 } };  // TODO: check 2nd dimension array size

//Sensors:
//============

#define SENSOR_READ_INTERVAL 30000  //long value in millis

//SD-Card-Logger:
//TODO

//Soil Humidity sensors:
class SoilWatering;              //Declare here for compiler
SoilWatering soilWatering;       //declare general instance to use.
#define soilNodesRngStart 1;     // TODO!! : Make sure these are correct!
#define needsWateringBelow 511;  //TODO : test needed value + do we want individual setting section?

//Temp/Humid/Pressure sensor:
BME280I2C bme;


void setup() {
  //DEBUGGING:
  //========
  pinMode(LED_BUILTIN, OUTPUT);  //For debugging
  Serial.begin(SERIAL_BAUD);     //For debugging
  //Serial.println(F("Serial test"));

  while (!Serial) {}  // Wait

  //SENSORS:
  //========
  soilWatering.begin(soilNodesRngStart, needsWateringBelow, &Serial)
    Wire.begin();

  while (!bme.begin()) {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }

  switch (bme.chipModel()) {
    case BME280::ChipModel_BME280:
      Serial.println("Found BME280 sensor! Success.");
      break;
    case BME280::ChipModel_BMP280:
      Serial.println("Found BMP280 sensor! No Humidity available.");
      break;
    default:
      Serial.println("Found UNKNOWN sensor! Error!");
  }

  bme.begin();

  //UI Menu:
  //========
  //LCD:
  lcd.begin(16, 2);
  line1.reserve(17);
  line2.reserve(17);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_ENTER, INPUT);
}

void loop() {
  unsigned long currentMillis = millis();

  //MainMenu:
  //===========
  //Menu Buttons
  int activeMenuBtn = NULL;
  for (int i = BTN_RNG_START; i <= BTN_RNG_END; i++) {
    if (digitalRead(i) == HIGH) {
      activeMenuBtn = i;
    }
  }
  switch (activeMenuBtn) {
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
        currSubPageCount = subPageValuesRange[menuPage];  //Set the range of possible values for this sub page
        notInSubMenu = false;
      } else {
        printLcdText("Exited:" + menuPage, "");
        notInSubMenu = true;
      }
      delay(500);
      break;
  }

  //SensorHandling
  //==============

  //TODO: implement correct timing of:
  soilWatering.collectSoilHumidityValues();
  //and
  soilWatering.toggleWatering();


  if (currentMillis - previousMillis >= SENSOR_READ_INTERVAL) {
    previousMillis = currentMillis;  // saves current value, so that the time you ran this section can be checked
    storeBME280Data(&Serial);        //TODO: Add data Logger; to get started: https://github.com/arduino-libraries/SD/blob/master/examples/Datalogger/Datalogger.ino
  }
}
