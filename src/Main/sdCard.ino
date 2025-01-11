/**
*   returns pointer to the first preference out of an array of 6
*/
int* getPreferences(){
  return preferences;
}

/**
*   Change one preference at position i to a new value
*/
void setPreferences(int* new_val, int i){
  if(i < (sizeof(preferences)/sizeof(preferences[0]))){
    preferences[i] = *new_val;
  }
}

/**
*   Reads Preferences from SD card -- Currently not completely safe against wrong inputs!
*/
void readPreferences (){
  logger.d("Trying to read preferences...");
  File prefFile = SD.open("pref.txt", FILE_READ);

  if(prefFile){
    char input;     // Defines that our input is of the 'char' format.
    int digit;      // The numerical storage of our char.
    int buff[10];   // Our buffer storing our digits before we can combine them into the result
    int prefnr = 0; // Which preference we are currently reading
    int powr = 0;   // Counts up to see how many digits are in a number
    int result = 0; // The output
    int pow10[10] = {1, 10, 100, 1000, 10000, 
        100000, 1000000, 10000000, 100000000, 1000000000};  // Using an array like this is slightly faster than using pow and does not require a library

    while(prefFile.available()){
      //set our input as a char
      input = prefFile.read();
      //All digits in ASCII are in consecutive order, by subtracting '0' or 48 we get the actual value. ',' will be negative here.
      digit = input - '0';

      //if it is a digit add to buffer
      if (digit >= 0){
        buff[powr] = digit;
        powr++;
      }else{
        powr--;
        //go over all digits in the buffer and combine them into the result.
        for(int i = 0; i <= powr; i++){
          result += buff[powr-i] * pow10[i];
        }
        preferences[prefnr] = result;
        prefnr++;
        //reset the buffer for new digits.
        memset(buff,0, sizeof(buff));
        powr = 0;
        result = 0;
      }
      if(prefnr>12){
        logger.d("Exceeded preferences input limit, please check file again. Amount of preferences is 12");
        prefFile.close();
        return;
      }
    }
    
  } else {
    logger.c("ERROR: Could not open/create preferences-file to read preferences, is SD card unplugged?");
  }
  char buffer[128];
  sprintf(buffer, "Now stored preferences are: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, ", preferences[0], preferences[1],preferences[2],preferences[3],preferences[4],preferences[5],preferences[6],preferences[7],preferences[8],preferences[9],preferences[10],preferences[11]);
  logger.d(buffer);
  prefFile.close();
}

/**
*   Stores the current preferences into the SD card if available. This overwrites old preferences if present.
*/
void storePreferences (){
  //Remove old file
  SD.remove("pref.txt");

  //Create and open new file to write in
  File prefFile = SD.open("pref.txt", FILE_WRITE);
  if(prefFile){
    for(int i = 0; i < (sizeof(preferences)/sizeof(preferences[0])); i++){
      prefFile.print(preferences[i]);
      prefFile.print(",");
    }
  } else {
    logger.c("ERROR: Could not open/create preferences-file to store preferences, is SD card unplugged?");
  }
  prefFile.close();

}

/**
*   Takes sensor data and writes it onto a new line in the SD card
*/
void storeData(uint8_t* humidityData, float* bme280Data){
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