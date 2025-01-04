//Temperature + Air Humididty + Air Pressure:
//==========================================

/**
*  Store/Log sensor Data to SD-Card, returns pointer to the first float in an array with size 3
*/
float* getBME280Data(SimpleLogger& logger) {
  float temp(NAN), hum(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, temp, hum, tempUnit, presUnit);
  static float bme280Data[3] = {pres,temp,hum};
  //For Debugging:
  char buffer[128];
  sprintf(buffer, "\nTemp (°C): %.2f \nHumidity (RH): %.2f \nPressure (Pa): %.2f", temp, hum, pres);
  logger.d(buffer);
  
  return bme280Data;
}
