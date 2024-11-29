//TOOD : MOVE INTO EXTERNAL FILE + CORRECT IMPORT IN MAIN.

/**
*  Class containing 
*/
class SoilWatering {
private:
  //General variables to retrieve before everything elese with begin():
  int soilNodesRngStart;
  Stream* serial;

  //For main watering logic:
  int currentSoilHumidityAvrg[9] = { 0 };
  uint8_t currentAvrgIteration = 0;
  const int checkNeedsWateringEvery = 5;  //TODO: is to be tested, reduces issues if spike mesurments happen.
  bool isCurrentlyWatering;               // Used for toggle state

  //For the queue:
  uint8_t buffer[9] = { 0 };  // Use byte (uint8_t) instead of int for efficiency
  uint8_t head = 0;           // Next position to write
  uint8_t count = 0;          // Number of elements currently in buffer

  /**
    *  Add a cell value to the queue (0-9)
    */
  void queueAdd(uint8_t value) {
    if (value > 9) {
      serial->print("ERROR: Watering cell value must be between 0 and 9, but was" + ((int)value));
    }

    // Add the value at the current head position
    buffer[head] = value;

    // Move head and update count
    head = (head + 1) % 9;
    count = min(count + 1, 9);
  }

  /**
    *  Retrieve next value and advance or return null value: 10.
    */
  uint8_t queuGetNext() {
    if (count == 0) {
      return 10
    }

    // Calculate the index to retrieve
    // This is the oldest element (head - count + 9) % 9
    int retrieveIndex = (head - count + 9) % 9;
    uint8_t value = buffer[retrieveIndex];

    // Decrease count
    count--;

    return value;
  }

public:
  //Constructor:
  explicit SoilWatering();

  /**
    *  Used to retireve correct values at runtime
    */
  void begin()

    void collectSoilHumidityValues() {
    if (currentAvrgIteration == 0) {
      for (int i = 0; i < 9; i++) {
        currentSoilHumidityAvrg[i] = analogRead(soilNodesRngStart + i);
      }
    } else {
      for (int i = 0; i < 9; i++) {
        currentSoilHumidityAvrg[i] = (currentSoilHumidityAvrg[i] + analogRead(soilNodesRngStart + i)) / 2;  //Set to average of existing and new value.
      }
      if (currentAvrgIteration == checkNeedsWateringEvery - 1) {
        for (int i = 0; i < 9; i++) {
          if (currentSoilHumidityAvrg[i] < neesWateringBelow) {
          }
        }
      }
    }
    currentAvrgIteration = (currentAvrgIteration + 1) % checkNeedsWateringEvery;
  }

  /**
    * Call in the intervall which corresponds to the duration, that a plant sould be watered every time. 
    */
  void toggleWatering() {
    if (isCurrentlyWatering) {
      //TODO : close valve.
      //TODO : recursive call this with next in queue or terminate if it was null element (10).
    }
    //TODO : perform watering start init here (i.e. move -> open valve). Try to implement without blocking the whole program flow. 
  }

  void forceStop() {
    //TODO : implement for emergency interrupt. Make sure system isn't corrupted afterwards...
  }
};