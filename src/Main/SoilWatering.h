#ifndef SOILWATERING_H  // Include guard to prevent multiple inclusions
#define SOILWATERING_H

#include <Arduino.h>
#include <SimpleLogger.h>

class SoilWatering {
private:
  // General variables
  int soilNodesRngStart;
  int needsWateringBelow;
  SimpleLogger* logger = nullptr;

  // Main watering logic variables
  int currentSoilHumidityAvrg[9] = { 0 };
  uint8_t currentAvrgIteration = 0;
  const int checkNeedsWateringEvery = 5;
  // bool isCurrentlyWatering = false;
  int wateringDuration = 10000; // Letting water flow for 10 seconds
  int gridWidth = 3; // 3x3 grid to calculate positions of plant

  // Watering queue variables
  uint8_t buffer[9] = { 0 };
  uint8_t head = 0;
  uint8_t count = 0;

  // Private queue functions
  void queueAdd(uint8_t value);
  uint8_t queueGetNext();

  // Private watering functions
  void moveTo();
  void mapPosition();
  void openValve();
  void closevalve();
  

public:
  // Constructor
  explicit SoilWatering();

  // Public methods
  void begin(int soilNodesRngStart, int needsWateringBelow, SimpleLogger& logger);
  uint8_t* collectSoilHumidityValues();
  void toggleWatering();
  void forceStop();
};

#endif  // SOILWATERING_H