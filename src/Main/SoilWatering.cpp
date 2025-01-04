#include "SoilWatering.h"

SoilWatering::SoilWatering() {
  // empty constructor
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

  // TODO: Implement stepper logic

  logger->d("Arrived at position (" + String(x) + ", " + String(y) + ").");
}

void SoilWatering::mapPosition(int index, int& x, int& y) {
  // TODO think of a good way to map index to motor position
  // Could be like this
  if (index == 0){
    x = 0;
    y = 0;
  }
  // Or linear mapping like
  row = index / gridWidth;     // Determine the row
  column = index % gridWidth;  // Determine the column
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

// Emergency stop function
void SoilWatering::forceStop() {
  // TODO: Implement emergency interrupt logic
}