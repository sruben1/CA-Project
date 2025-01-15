/**
*  Truns on and off internal led for Debug purposes
*/
void debugLed() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on
  delay(100);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

/**
*  Basic debug helper method to log a long unsigned variable and a char* message at debug level. 
*  Warning, implemented with 128 char buffer (max lenth is fairly limited).
*/
void logLongUnsigned(const char* message, const unsigned long variable) {
  char buffer[128];
  snprintf(buffer, sizeof(buffer),"%s%lu",  message, variable);
  logger.d(buffer);
}

void logExportFunction(const char* message, uint8_t level){
  switch (level){
    case LOG_LEVEL_CRITICAL:
      logger.c(message);
      break;
    case LOG_LEVEL_WARNING:
      logger.w(message);
      break;
    case LOG_LEVEL_INFO:
      logger.i(message);
      break;
    case LOG_LEVEL_DEBUG:
      logger.d(message);
      break;
    default:
      logger.d("LOGGER ERROR, non valid level...");
      break;
  }
}
