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
uint8_t SoilWatering::queuGetNext() {
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
          // TODO: Implement watering logic
        }
      }
    }
  }
  currentAvrgIteration = (currentAvrgIteration + 1) % checkNeedsWateringEvery;
}

// Toggle watering based on the state
void SoilWatering::toggleWatering() {
  if (isCurrentlyWatering) {
    // TODO: Close valve logic
    // TODO: Recursive call with next in queue or terminate if null element (10)
  } else {
    // TODO: Start watering logic without blocking the program
  }
}

// Emergency stop function
void SoilWatering::forceStop() {
  // TODO: Implement emergency interrupt logic
}