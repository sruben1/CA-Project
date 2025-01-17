//Temperature + Air Humididty + Air Pressure and Ventilation:
//==========================================

/**
*  Measure sensor Data, returns pointer to the first float in an array with size 3, containing [Temperature, Humidity, Pressure]
*/
float* getBME280Data(SimpleLogger& logger) {
  float temp(NAN), hum(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, temp, hum, tempUnit, presUnit);
  static float bme280Data[3] = {pres,temp,hum};
  //For debugging:
  Serial.print("BME measurments: ");
  Serial.print(pres);
  Serial.print("Pa, ");
  Serial.print(temp);
  Serial.print("C, ");
  Serial.print(hum);
  Serial.println("%");
  
  return bme280Data;
}
/**
*   Turns the fan on if measured temperature/humidity exceeds maxAirTemperature/maxAirHumidity. Fan will never turn on if temperature is more than 5 degrees colder than the max value.
*/
void ventilationCheck(float* bmeData){
  //Check that the sensor is working and giving proper data back
  if(!(isnan(*(bmeData+1)) || isnan(*(bmeData+2)))){
    //check if values exceed the max
    if(((*(bmeData+1) > maxAirTemperature) || (*(bmeData+2) > maxAirHumidity)) && (*(bmeData+1) > maxAirTemperature-5)){
      digitalWrite(FAN,HIGH);
    }else{
      digitalWrite(FAN,LOW);
    }
  }else {
    logger.c("Could not read BME280Sensor-Data! Is sensor not connected?");
  }
}