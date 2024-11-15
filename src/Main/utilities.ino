unsigned unsignedModulo(int value, unsigned int m) {
    int mod = value % (int)m;
    if (mod < 0) {
        mod += m;
    }
    return mod;
}

/**
*  Truns on and off internal led for Debug purposes
*/
void debugLed(){
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}