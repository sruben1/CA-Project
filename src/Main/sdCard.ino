#include <SPI.h>
#include <SD.h>

//set Standart preferences:
static uint8_t preferences[6] = {0,0,0,0,0,0};

/**
*   returns pointer to the first preference out of an array of 6
*/
uint8_t* getPreferences(){
  return preferences;
}

/**
*   Change one preference at position i to a new value
*/
void setPreferences(uint8_t* new_val, int i){
  if(i < (sizeof(preferences)/sizeof(preferences[0]))){
    preferences[i] = *new_val;
  }
}

/**
*   Reads Preferences from SD card
*/
void readPreferences (SimpleLogger& logger){
  File prefFile = SD.open("preferences.txt", FILE_READ);

  if(prefFile){
    while(prefFile.available()){
      //TODO: define how preferences look like and how they are read
    }
  } else {
    logger.c("ERROR: Could not open/create preferences-file, is SD card unplugged?");
  }
  prefFile.close();
}

/**
*   Stores the current preferences into the SD card if available. This overwrites old preferences if present.
*/
void storePreferences (SimpleLogger& logger){
  //Remove old file
  SD.remove("preferences.txt");

  //Create and open new file to write in
  File prefFile = SD.open("preferences.txt", FILE_WRITE);
  if(prefFile){
    for(int i = 0; i < (sizeof(preferences)/sizeof(preferences[0])); i++){
      prefFile.print(preferences[i]);
      prefFile.print(",");
    }
  } else {
    logger.c("ERROR: Could not open/create preferences-file, is SD card unplugged?");
  }
  prefFile.close();

}

/**
*   Takes sensor data and writes it onto a new line in the SD card
*/
void storeData(uint8_t* humidityData, float* bme280Data, SimpleLogger& logger){
  //Open the file, if there is no file, a new one is created
  File dataFile = SD.open("data.txt", FILE_WRITE);

  //Check if the file is still here -> fails if there is no SD Card
  if(dataFile){
    dataFile.print(*humidityData);
    dataFile.print(",");

    for (int i = 0; i < 4; i++){
      dataFile.print(*(bme280Data+i));
      dataFile.print(",");
    }
    dataFile.println("");
  } else {
    logger.c("ERROR: Could not open/create data-file, is SD card unplugged?");
  }
  
  //close the file at the end
  dataFile.close();
}