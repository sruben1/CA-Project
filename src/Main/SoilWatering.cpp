#include "SoilWatering.h"

// Constructor definition
SoilWatering::SoilWatering() {
  // Initialize values if needed
}

// Initialize soil monitoring settings
void SoilWatering::begin(int soilNodesRngStart, int needsWateringBelow, Stream* serial) {
  this->soilNodesRngStart = soilNodesRngStart;
  this->needsWateringBelow = needsWateringBelow;
  this->serial = serial;
}

// Add a value to the queue
void SoilWatering::queueAdd(uint8_t value) {
  if (value > 9) {
    serial->println("ERROR: Watering cell value must be between 0 and 9, but was " + String((int)value));
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

// Collect and update soil humidity values
void SoilWatering::collectSoilHumidityValues() {
  if (currentAvrgIteration == 0) {
    for (int i = 0; i < 9; i++) {
      currentSoilHumidityAvrg[i] = analogRead(soilNodesRngStart + i);
    }
  } else {
    for (int i = 0; i < 9; i++) {
      currentSoilHumidityAvrg[i] = (currentSoilHumidityAvrg[i] + analogRead(soilNodesRngStart + i)) / 2;
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