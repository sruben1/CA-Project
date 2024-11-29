//Temperature + Air Humididty + Air Pressure:
//==========================================

/**
*  Store/Log sensor Data to SD-Card
*/
void storeBME280Data(Stream* serial) {
  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, temp, hum, tempUnit, presUnit);

  //For Debugging:
  serial->print("Temp: ");
  serial->print(temp);
  serial->print("°" + String(tempUnit == BME280::TempUnit_Celsius ? 'C' : 'F'));
  serial->print("\t\tHumidity: ");
  serial->print(hum);
  serial->print("% RH");
  serial->print("\t\tPressure: ");
  serial->print(pres);
  serial->println("Pa");

  delay(1000);  //TODO: can remove?

  //TODO : Write Data to SD-Card
}
