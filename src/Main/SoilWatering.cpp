#include "SoilWatering.h"
#include "AccelStepper.h"
#include <Servo.h>

SoilWatering::SoilWatering() {
}

/**
*  Getting setup variables from main
*/
void SoilWatering::begin(int soilNodesRngStart, const int* moistureLevels, int wateringDuration, LogFunction log, int motorPinX1, int motorPinX3, int motorPinX2, int motorPinX4, int motorPinY1, int motorPinY3, int motorPinY2, int motorPinY4, int servoPin, int HOME_SWITCH_PIN_X, int HOME_SWITCH_PIN_Y) {
  this->soilNodesRngStart = soilNodesRngStart;
  this->soilMoistureLevels = moistureLevels;
  this->wateringDuration = wateringDuration; 
  this->logFn = log;

  this->motorPin1X = motorPinX1;
  this->motorPin2X = motorPinX2;
  this->motorPin3X = motorPinX3;
  this->motorPin4X = motorPinX4;
  this->motorPin1Y = motorPinY1;
  this->motorPin2Y = motorPinY2;
  this->motorPin3Y = motorPinY3;
  this->motorPin4Y = motorPinY4;

  this->HOME_SWITCH_PIN_X = HOME_SWITCH_PIN_X;
  this->HOME_SWITCH_PIN_Y = HOME_SWITCH_PIN_Y;

  // Initialize with pin sequence IN1-IN3-IN2-IN4! Please do not change! (Or ask Luis)
  stepperX = AccelStepper(MotorInterfaceType, motorPin1X, motorPin3X, motorPin2X, motorPin4X);
  stepperY = AccelStepper(MotorInterfaceType, motorPin1Y, motorPin3Y, motorPin2Y, motorPin4Y);

  // These settings are not passed from main as they don't have to be changed after calibrating of settings has been done.
  stepperX.setMaxSpeed(maxSpeed);      // Maximum steps per second
  stepperX.setAcceleration(acceleration);  // Steps per second squared
  stepperY.setMaxSpeed(maxSpeed);      // Maximum steps per second
  stepperY.setAcceleration(acceleration);  // Steps per second squared

  // Setting the current position of the stepper motors as 0. 
  // TODO: Implement homing sequence if we have time left
  stepperX.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);

  logD("SoilWatering class now is intialized with variabel parameters.");

  servo.attach(servoPin);
  servo.write(88);
}

/**
*  Add a value to the queue
*/
void SoilWatering::queueAdd(uint8_t value) {
  //Skip if already in queue:
  if (getInQueueState(value)){
    logUnsignedDebug("INFO: Cell nbr %u skipped since status set as alreay in queue.", value);
    return;
  }

  if (value > 9) {
    logUnsignedDebug("ERROR: Watering cell value must be between 0 and 9, but was %u", value);
    return;
  }

  buffer[head] = value;
  head = (head + 1) % 9;
  count = min(count + 1, 9);
  setInQueueState(value, true);
}

/**
*  Retrieve the next value from the queue
*/
uint8_t SoilWatering::queueGetNext() {
  if (count == 0) {
    return 10;  // Null value
  }

  int retrieveIndex = (head - count + 9) % 9;
  uint8_t value = buffer[retrieveIndex];
  count--;
  setInQueueState(value, false);

  return value;
}

/**
*  Get state of one possible queue element (0 to 8) (used for duplciate check):
*/
bool SoilWatering::getInQueueState(int index){
    if (index < 0 || index > 8) return false;  // Out-of-bounds check
  
  return (isInQueue & (1 << index)) != 0;  // Check if the bit is set at the specified index
}

/**
*  Set state of one possible queue element (0 to 8) (used for duplciate check):
*/
void SoilWatering::setInQueueState(int index, bool value){
  if (index < 0 || index > 8){ // Out-of-bounds check
    logUnsignedDebug("Out of bounds value %u catched at duplicate check!", index);
    return;  
  }

  if (value) {
      isInQueue |= (1 << index);  // Set the bit at the specified index
  } else {
      isInQueue &= ~(1 << index); // Clear the bit at the specified index
  }
}

/** 
*  Collect and update soil humidity values and return an array of length 9.
*/
uint8_t* SoilWatering::collectSoilHumidityValues() {
  static uint8_t returnValues[9] = {0};
  if (currentAvrgIteration == 0) {
    logD("In Oth average iteration, overwiriting array with first values...");
    for (int i = 0; i < 9; i++) {
      returnValues[i] = analogRead(soilNodesRngStart + i);
      currentSoilHumidityAvrg[i] = returnValues[i];
    }
  } else {
    for (int i = 0; i < 9; i++) {
      returnValues[i] = analogRead(soilNodesRngStart + i);
      logIntegerDebug("Sensor %d value %d", i, returnValues[i]);
      currentSoilHumidityAvrg[i] = (currentSoilHumidityAvrg[i] + returnValues[i]) / 2;
    }
    if (currentAvrgIteration == checkNeedsWateringEvery - 1) {
      logD("In 5th average iteration, filling queue...");
      for (int i = 0; i < 9; i++) {
        if (currentSoilHumidityAvrg[i] < soilMoistureLevels[i]) {
          logIntegerDebug("Plant at position %d needs watering.", i, 0);
          queueAdd(i); // Add plant's position to the watering queue
        }
      }
    }
  }
  currentAvrgIteration = (currentAvrgIteration + 1) % checkNeedsWateringEvery;
  return returnValues;
}

/**
*  Logic for XY-Movement and watering
*/
void SoilWatering::toggleWatering() {
  uint8_t nextValue = queueGetNext();
  // TODO: Decide if we want a homing or not -> can be done without limit siwitches as motors are to weak
  if (nextValue == 10) {
      logD("No plant to water. Stopping.");
      return;
  }
  logUnsignedDebug("Watering at position %u", nextValue);
  moveTo(nextValue);
  openValve();
  delay(wateringDuration);  // Wait for the watering to complete
  closeValve();
  //moveTo(10); Home position of XY-Table EDIT: Removed after iterative approach each cycle
}

/**
*  Move the XY-Table to the correct position
*/
void SoilWatering::moveTo(uint8_t arrayPosition) {
  if (arrayPosition > 9) {
  logFn("Invalid arrayPosition passed to moveTo.", LOG_LEVEL_WARNING);
  return;
}
// TODO: Ask Shura if i can just replace this uint8_t with a long? -> it's needed here as int is to short
  long x, y;

  if (arrayPosition == 10){ // Homing of positions
    x = 0;
    y = 0;
  } else{
    mapPosition(arrayPosition, x, y); // Map the position to the plant grid
  }

  logIntegerDebug("Moving to position (%d, %d).", x, y);

  // Define where to move to
  stepperX.moveTo(x);
  stepperY.moveTo(y);

  // Move in x and y direction
  while (stepperX.distanceToGo() != 0 || stepperY.distanceToGo() != 0) {
    stepperX.run();  // Run the motor to the target position
    stepperY.run();
  }
  stepperX.stop();
  stepperY.stop();

  logIntegerDebug("Arrived at position (%d, %d).", x, y);
}

/**
*  Translate the array position to a 2D-Coordinate
*/
void SoilWatering::mapPosition(int index, long& x, long& y) {
  // Warning: Array index is 0 indexed but plant position 1 indexed
  if (index == 0){
    x = 4000;
    y = 4000;
  }
  if (index == 1){
    x = 4000;
    y = 19000;
  }
  if (index == 2){
    x = 4000;
    y = 34000;
  }
  if (index == 3){
    x = 19000;
    y = 4000;
  }
  if (index == 4){
    x = 19000;
    y = 19000;
  }
  if (index == 5){
    x = 19000;
    y = 34000;
  }
  if (index == 6){
    x = 34000;
    y = 4000;
  }
  if (index == 7){
    x = 34000;
    y = 19000;
  }
  if (index == 8){
    x = 34000;
    y = 34000;
  }

  /*
  // Or linear mapping like
  x = index / gridWidth;     // Determine the row
  y = index % gridWidth;  // Determine the column

  // Add multiplier and offset from 0 0
  x = x*15000 + 4000; // Ex: x=0 -> xPos 4000, x = 1 -> xPos 19000
  y = y*15000 + 4000;
  */
}

/**
*  Opening of the valve
*/
void SoilWatering::openValve() {
  logD("Opening valve.");
  servo.write(0);
  delay(250);
}

/**
*  Closing of the valve
*/
void SoilWatering::closeValve() {
  logD("Closing valve.");
  servo.write(88);
  delay(250);
}

// Logic to home steppers example:
void SoilWatering::homeStepper() {
  logD("Starting homing");
  logUnsignedDebug("Switch X: %d", digitalRead(HOME_SWITCH_PIN_X) == LOW);
  logUnsignedDebug("Switch Y: %d", digitalRead(HOME_SWITCH_PIN_Y) == LOW);
  stepperX.setSpeed(-500);  // Move in the direction towards the home switch
  stepperY.setSpeed(-500);

  // Move the stepper until the limit switch is triggered
  logD("Homing X");
  while (digitalRead(HOME_SWITCH_PIN_X) == LOW) {
    stepperX.runSpeed();
    logD("Moving X");
  }
  stepperX.stop();
  stepperX.setCurrentPosition(0);
  stepperX.moveTo(100);
  stepperX.setCurrentPosition(0);
  while (stepperX.distanceToGo() != 0) {
    stepperX.run();
  }
  delay(100);

  logD("Homing Y");
  /*
  while (digitalRead(HOME_SWITCH_PIN_Y) == LOW) {
    stepperY.runSpeed();
    logD("Moving Y");
  }
  */
  stepperY.stop();
  stepperY.setCurrentPosition(0);
  stepperY.moveTo(100);
  while (stepperY.distanceToGo() != 0) {
    stepperY.run();
  }
  stepperY.setCurrentPosition(0);
  delay(100);
  logD("Finished Homing");


  stepperX.setSpeed(maxSpeed);
  stepperY.setSpeed(maxSpeed);
}

void SoilWatering::demo() {

}

// Emergency stop function
void SoilWatering::forceStop() {
  // TODO: Implement emergency interrupt logic
  stepperX.stop();
  stepperY.stop();
}

void SoilWatering::logUnsignedDebug(const char* format, const unsigned value){
  static char buffer[128];
  snprintf(buffer, sizeof(buffer), format, value);
  logFn(buffer, LOG_LEVEL_DEBUG);
}

void SoilWatering::logIntegerDebug(const char* format, const int value1, const int value2){
  static char buffer[128];
  snprintf(buffer, sizeof(buffer), format, value1, value2);
  logFn(buffer, LOG_LEVEL_DEBUG);
}

void SoilWatering::logD(const char* message){
  logFn(message, LOG_LEVEL_DEBUG);
}