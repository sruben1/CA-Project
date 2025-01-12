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
  int* soilMoistureLevels;

  // Main watering logic variables
  int currentSoilHumidityAvrg[9] = { 0 };
  uint8_t currentAvrgIteration = 0;
  const int checkNeedsWateringEvery = 5;
  // bool isCurrentlyWatering = false;
  int wateringDuration = 3000; // Letting water flow for 3 seconds
  int gridWidth = 3; // 3x3 grid to calculate positions of plant

  // Watering queue variables
  uint8_t buffer[9] = { 0 };
  uint8_t head = 0;
  uint8_t count = 0;

  // Stepper variables
  AccelStepper stepperX;
  AccelStepper stepperY;

  int motorPin1X;
  int motorPin2X;
  int motorPin3X;
  int motorPin4X;

  int motorPin1Y;
  int motorPin2Y;
  int motorPin3Y;
  int motorPin4Y;

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
  void demo();
};

#endif  // SOILWATERING_H