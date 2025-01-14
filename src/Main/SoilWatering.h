#ifndef SOILWATERING_H  // Include guard to prevent multiple inclusions
#define SOILWATERING_H

#include <Arduino.h>
#include <SimpleLogger.h>
#include <AccelStepper.h>

class SoilWatering {
private:
  // General variables
  int soilNodesRngStart;
  // int needsWateringBelow;
  SimpleLogger* logger = nullptr;
  int* soilMoistureLevels;

  // Main watering logic variables
  int currentSoilHumidityAvrg[9] = { 0 };
  uint8_t currentAvrgIteration = 0;
  const int checkNeedsWateringEvery = 5;
  // bool isCurrentlyWatering = false;
  int wateringDuration = 3000; // Letting water flow for 3 seconds
  #define gridWidth 3 // 3x3 grid to calculate positions of plant

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

  #define MotorInterfaceType 8 //4 wire motor in half step mode

  #define maxSpeed 650 // Maximum allowed steps per second
  #define acceleration 250

  // Private queue functions
  void queueAdd(uint8_t value);
  uint8_t queueGetNext();

  // Private watering functions
  void openValve();
  void closeValve();

  // Stepper functions
  void moveTo(uint8_t arrayPosition);
  void mapPosition(int index, uint8_t& x, uint8_t& y);


  public:
  // Constructor
  explicit SoilWatering();

  // Public methods
  void SoilWatering::begin(int soilNodesRngStart, int* moistureLevels, SimpleLogger& logger, int motorPinX1, int motorPinX3, int motorPinX2, int motorPinX4, int motorPinY1, int motorPinY3, int motorPinY2, int motorPinY4);
  uint8_t* collectSoilHumidityValues();
  void toggleWatering();
  void forceStop();
  void demo();
};

#endif  // SOILWATERING_H