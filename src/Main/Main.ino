#include <LiquidCrystal.h>
#include <BME280I2C.h>  // https://github.com/finitespace/BME280
#include <Wire.h>       // (I2C)
#include <SPI.h>        // Used for SD-Card
#include <SD.h>         //https://github.com/arduino-libraries/SD

#define SERIAL_BAUD 115200 // Serial frequency

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
const int btnLeft = 2;
const int btnRight = 3;
const int btnEnter = 4;
// => Range of these buttons:
const int btnRngStart = 2;
const int btnRngEnd = 4;

//Main menu page selctor:
bool notInSubMenu = true;
int menuPage = 0;
const unsigned int menuPageCount = 5;
const char* mainPageNames[menuPageCount] = { "First", "Seccond", "Third", "Fourth", "Fifth" };

//Sub menu implementation:
int subPageValue = 0;
int currSubPageCount = 1;                                                                                      //init with dummy value
const int subPageValuesRange[menuPageCount] = { 3, 3, 3, 3, 5 };                                               // top range of parameterable sub page values
const int subPageNameAssignment[menuPageCount]{ 0, 0, 0, 0, 1 };                                               //Which naming scheme is used for the subpage variables, selects index of "subPageStandardValueNames"
const char* subPageStandardValueNames[menuPageCount][7] = { { "Yes", "No", "Cancel" }, { 0, 1, 2, -2, -1 } };  // TODO: check 2nd dimension array size

//Sensors:
//============

const long sensorReadInterval = 30000;  //in millis

//SD-Card-Logger:
//TODO

//Soil Humidity sensors:
class SoilWatering;               //Declare here for compiler
SoilWatering soilWatering;        //declare general instance to use.
const int soilNodesRngStart = 1;  // TODO!! : Make sure these are correct!

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
  pinMode(btnLeft, INPUT);
  pinMode(btnRight, INPUT);
  pinMode(btnEnter, INPUT);
}

void loop() {
  unsigned long currentMillis = millis();

  //MainMenu:
  //===========
  //Menu Buttons
  int activeMenuBtn = NULL;
  for (int i = btnRngStart; i <= btnRngEnd; i++) {
    if (digitalRead(i) == HIGH) {
      activeMenuBtn = i;
    }
  }
  switch (activeMenuBtn) {
    case btnLeft:
      if (notInSubMenu) {
        menuPage = unsignedModulo(menuPage - 1, menuPageCount);
        printLcdText("On Page:" + String(menuPage), "");
      } else {
        subPageValue = unsignedModulo(subPageValue - 1, currSubPageCount);
        printLcdText("Page:" + String(menuPage), "On Sub Value:" + String(subPageValue));
      }
      delay(500);
      break;
    case btnRight:
      if (notInSubMenu) {
        menuPage = unsignedModulo(menuPage + 1, menuPageCount);
        printLcdText("On Page:" + menuPage, "");
      } else {
        subPageValue = unsignedModulo(subPageValue + 1, currSubPageCount);

        printLcdText("Page:", menuPage, "On Sub Value:", subPageValue);
      }
      delay(500);
      break;
    case btnEnter:
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

  //TODO: interpret soil sensors calls


  if (currentMillis - previousMillis >= sensorReadInterval) {
    previousMillis = currentMillis;  // saves current value, so that the time you ran this section can be checked
    storeBME280Data(&Serial);        //TODO: Add data Logger; to get started: https://github.com/arduino-libraries/SD/blob/master/examples/Datalogger/Datalogger.ino
  }
}
