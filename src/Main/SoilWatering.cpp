#include "SoilWatering.h"
#include "AccelStepper.h"

SoilWatering::SoilWatering() {
}

// Sets the constants to be used
void SoilWatering::begin(int soilNodesRngStart, int* moistureLevels, SimpleLogger& logger, int motorPinX1, int motorPinX3, int motorPinX2, int motorPinX4, int motorPinY1, int motorPinY3, int motorPinY2, int motorPinY4) {
  this->soilNodesRngStart = soilNodesRngStart;
  // this->needsWateringBelow = needsWateringBelow;
  this->soilMoistureLevels = moistureLevels;
  this->logger = &logger;

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
}

// Add a value to the queue
void SoilWatering::queueAdd(uint8_t value) {
  if (value > 9) {
    char buffer[64];
    sprintf(buffer, "ERROR: Watering cell value must be between 0 and 9, but was %u", value);
    logger->w(buffer);
    return;
  }

  buffer[head] = value;
  head = (head + 1) % 9;
  count = min(count + 1, 9);
}

// Retrieve the next value from the queue
uint8_t SoilWatering::queueGetNext() {
  if (count == 0) {
    return 10;  // Null value
  }

  int retrieveIndex = (head - count + 9) % 9;
  uint8_t value = buffer[retrieveIndex];
  count--;

  return value;
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
          logger->d(("Plant at position " + String(i) + " needs watering.").c_str());
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
      logger->i("No more plants to water. Stopping.");
      // break;  // Exit the loop if no more plants need watering EDIT: Not needed anymore because no while loop anymore
      return;
  }
  logger->d(("Watering at position " + String(nextValue)).c_str());
  moveTo(nextValue);
  openValve();
  delay(wateringDuration);  // Wait for the watering to complete
  closeValve();
  //moveTo(10); Home position of XY-Table EDIT: Removed after iterative approach each cycle
}

void SoilWatering::moveTo(uint8_t arrayPosition) {
  if (arrayPosition > 9) {
  logger->w("Invalid arrayPosition passed to moveTo.");
  return;
}
  uint8_t x, y;

  if (arrayPosition == 10){ // Homing of positions
    x = 0;
    y = 0;
  } else{
    mapPosition(arrayPosition, x, y); // Map the position to the plant grid
  }

  logger->d(("Moving to position (" + String(x) + ", " + String(y) + ").").c_str());

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

  logger->d(("Arrived at position (" + String(x) + ", " + String(y) + ").").c_str());
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
  logger->d("Opening valve.");
  // servo.attach(VALVE_PIN);  // Attach the servo to the valve pin (VALVE_PIN is a predefined constant)
  // servo.write(VALVE_OPEN_ANGLE);  // Move servo to the angle that opens the valve
  // delay(500);  // Small delay to allow the servo to physically move
}

void SoilWatering::closeValve() {
  // TODO: Implemet according to the servo we have. Example implementation:
  logger->d("Closing valve.");
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