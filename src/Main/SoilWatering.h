#ifndef SOILWATERING_H  // Include guard to prevent multiple inclusions
#define SOILWATERING_H

#include <Arduino.h>
#include <SimpleLogger.h>
#include <AccelStepper.h>

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

  // Stepper variables
  // TODO: Replace with correct pins
  constexpr int motorPin1X = 22;
  constexpr int motorPin2X = 24;
  constexpr int motorPin3X = 26;
  constexpr int motorPin4X = 28;

  constexpr int motorPin1Y = 23;
  constexpr int motorPin2Y = 25;
  constexpr int motorPin3Y = 27;
  constexpr int motorPin4Y = 29;

  constexpr int MotorInterfaceType = 8; //4 wire motor in half step mode

  constexpr int maxSpeed = 650; // Maximum allowed steps per second
  constexpr int acceleration = 250;

  // Private queue functions
  void queueAdd(uint8_t value);
  uint8_t queueGetNext();

  // Private watering functions
  void openValve();
  void closevalve();

  // Stepper functions
  void moveTo();
  void mapPosition();


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