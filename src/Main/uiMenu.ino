/**
*  Overloaded print method (simple dual line input):
*/
void printLcdText(const String& line1, const String& line2) {
  lcd.clear();
  delay(100);  // Used to avoid command overflow.

  // Print the text to the LCD display
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
/**
*  Display on the screen that system is busy
*/
void startBusy() {
  //set status LEDs:
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);


  lcd.clear();
  delay(100);
  // Print the info text to the LCD display
  lcd.setCursor(0, 0);
  lcd.print("System is Busy...");
  lcd.setCursor(0, 1);
  lcd.print("Please wait.");
}

void endBusy(){
    //set status LEDs:
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);

  lcd.clear();
  delay(100);
  // Print the info text to the LCD display
  lcd.setCursor(0, 0);
  lcd.print("Read again!");
  lcd.setCursor(0, 1);
  lcd.print("Use: + or -");
}

void downButtonInterrupt() {
   if(nextMenuBtnToHandle != BTN_ENTER){ // make sure enter case has highest priority
    nextMenuBtnToHandle = BTN_DOWN;
   }
}
void upButtonInterrupt() {
  if(nextMenuBtnToHandle != BTN_ENTER){ // make sure enter case has highest priority
    nextMenuBtnToHandle = BTN_UP;
  }
}
void enterButtonInterrupt() {
    nextMenuBtnToHandle = BTN_ENTER;
}

void prepSystemToShutDown(){
  shutDownNextIteration = true;
}