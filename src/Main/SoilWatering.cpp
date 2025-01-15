#include "SoilWatering.h"
#include "AccelStepper.h"

SoilWatering::SoilWatering() {
}

// Sets the constants to be used
void SoilWatering::begin(int soilNodesRngStart, const int* moistureLevels, int wateringDuration, LogFunction log, int motorPinX1, int motorPinX3, int motorPinX2, int motorPinX4, int motorPinY1, int motorPinY3, int motorPinY2, int motorPinY4) {
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
    for (int i = 0; i < 9; i++) {
      returnValues[i] = analogRead(soilNodesRngStart + i);
      currentSoilHumidityAvrg[i] = returnValues[i];
    }
  } else {
    for (int i = 0; i < 9; i++) {
      returnValues[i] = analogRead(soilNodesRngStart + i);
      currentSoilHumidityAvrg[i] = (currentSoilHumidityAvrg[i] + returnValues[i]) / 2;
    }
    if (currentAvrgIteration == checkNeedsWateringEvery - 1) {
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

// Toggle watering based on the state
void SoilWatering::toggleWatering() {
  uint8_t nextValue = queueGetNext();
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

void SoilWatering::moveTo(uint8_t arrayPosition) {
  if (arrayPosition > 9) {
  logFn("Invalid arrayPosition passed to moveTo.", LOG_LEVEL_WARNING);
  return;
}
  uint8_t x, y;

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

  // Move in x direction
  while (stepperX.distanceToGo() != 0) {
    stepperX.run();  // Run the motor to the target position
  }

  // Move in y direction
  while (stepperY.distanceToGo() != 0) {
    stepperY.run();  // Run the motor to the target position
  }

  logIntegerDebug("Arrived at position (%d, %d).", x, y);
}

void SoilWatering::mapPosition(int index, uint8_t& x, uint8_t& y) {
  // TODO think of a good way to map index to motor position
  // Could be like this
  /*
  if (index == 0){
    x = 0;
    y = 0;
  }
  */
  // Or linear mapping like
  x = index / gridWidth;     // Determine the row
  y = index % gridWidth;  // Determine the column

  // Add multiplier and offset from 0 0
  x = x*2048 + 2048; // Ex: x=0 -> xPos 2048, x = 1 -> xPos 4096
  y = y*2048 + 2049;
}

void SoilWatering::openValve() {
  // TODO: Implemet according to the servo we have. Example implementation:
  logD("Opening valve.");
  // servo.attach(VALVE_PIN);  // Attach the servo to the valve pin (VALVE_PIN is a predefined constant)
  // servo.write(VALVE_OPEN_ANGLE);  // Move servo to the angle that opens the valve
  // delay(500);  // Small delay to allow the servo to physically move
}

void SoilWatering::closeValve() {
  // TODO: Implemet according to the servo we have. Example implementation:
  logD("Closing valve.");
  // servo.write(VALVE_CLOSED_ANGLE);  // Move servo to the angle that closes the valve
  // delay(500);  // Small delay to allow the servo to physically move
  // servo.detach();  // Detach the servo to save power and prevent unwanted movements
}

// Logic to home steppers example:
/*
void SoilWatering::homeStepper() {
  stepperX.setSpeed(-200);  // Move in the direction towards the home switch

  // Move the stepper until the limit switch is triggered
  while (digitalRead(HOME_SWITCH_PIN) == HIGH) {
    stepperX.runSpeed();  // Run the motor at the set speed
  }

  // Stop the motor
  stepperX.stop();
  delay(100);  // Allow the motor to stop completely

  // Move slightly away from the switch to clear it
  stepperX.move(100);
  while (stepperX.isRunning()) {
    stepperX.run();
  }

  // Set the current position to 0
  stepperX.setCurrentPosition(0);
}
*/

void SoilWatering::demo() {

}

// Emergency stop function
void SoilWatering::forceStop() {
  // TODO: Implement emergency interrupt logic
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