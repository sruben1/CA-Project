#include "SoilWatering.h"
#include "AccelStepper.h"

AccelStepper stepperX;
AccelStepper stepperY;

SoilWatering::SoilWatering() {
  // Initialize with pin sequence IN1-IN3-IN2-IN4! Please do not change! (Or ask Luis)
  stepperX = AccelStepper(MotorInterfaceType, motorPinX1, motorPinX3, motorPinX2, motorPinX4);
  stepperY = AccelStepper(MotorInterfaceType, motorPinY1, motorPinY3, motorPinY2, motorPinY4);

  stepperX.setMaxSpeed(maxSpeed);      // Maximum steps per second
  stepperX.setAcceleration(acceleration);  // Steps per second squared
  stepperY.setMaxSpeed(maxSpeed);      // Maximum steps per second
  stepperY.setAcceleration(acceleration);  // Steps per second squared

  // Setting the current position of the stepper motors as 0. 
  // TODO: Implement homing sequence if we have time left
  stepperX.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);
}

// Sets the constants to be used from  
void SoilWatering::begin(int soilNodesRngStart, int needsWateringBelow, SimpleLogger& logger) {
  this->soilNodesRngStart = soilNodesRngStart;
  this->needsWateringBelow = needsWateringBelow;
  this->logger = &logger; 
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
        if (currentSoilHumidityAvrg[i] < needsWateringBelow) {
          logger->d("Plant at position " + String(i) + " needs watering.");
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
  while (true) {
    uint8_t nextValue = queueGetNext();
    if (nextValue == 10) {
        logger->i("No more plants to water. Stopping.");
        break;  // Exit the loop if no more plants need watering
    }
    logger->d("Watering at position "+String(nextValue));
    moveTo(nextValue);
    openValve();
    delay(wateringDuration);  // Wait for the watering to complete
    closeValve();
  }
  moveTo(10); // Home position of XY-Table
}

void SoilWatering::moveTo(int arrayPosition) {
  int x, y;

  if (arrayPosition == 10){ // Homing of positions
    x = 0;
    y = 0;
  } else{
    mapPosition(arrayPosition, x, y); // Map the position to the plant grid
  }

  logger->d("Moving to position (" + String(x) + ", " + String(y) + ").");

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

  logger->d("Arrived at position (" + String(x) + ", " + String(y) + ").");
}

void SoilWatering::mapPosition(int index, int& x, int& y) {
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
  x = x*2048 + 2048 // Ex: x=0 -> xPos 2048, x = 1 -> xPos 4096
  y = y*2048 + 2049
}

void SoilWatering::openValve() {
  // TODO: Implemet according to the servo we have. Example implementation:
  logger->d("Opening valve.");
  servo.attach(VALVE_PIN);  // Attach the servo to the valve pin (VALVE_PIN is a predefined constant)
  servo.write(VALVE_OPEN_ANGLE);  // Move servo to the angle that opens the valve
  delay(500);  // Small delay to allow the servo to physically move
}

void SoilWatering::closeValve() {
  // TODO: Implemet according to the servo we have. Example implementation:
  logger->d("Closing valve.");
  servo.write(VALVE_CLOSED_ANGLE);  // Move servo to the angle that closes the valve
  delay(500);  // Small delay to allow the servo to physically move
  servo.detach();  // Detach the servo to save power and prevent unwanted movements
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

// Emergency stop function
void SoilWatering::forceStop() {
  // TODO: Implement emergency interrupt logic
}