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
*  Overloaded print method (string and int value to concatenate for each line):
*/
void printLcdText(const String& str1l1, const int intl1, const String& str1l2, const int intl2) {
  lcd.clear();
  line1 = "";
  line2 = "";
  delay(100);
  //Concatenate stringd to assmble each line for display:
  line1 += str1l1;
  line1 += intl1;
  line2 += str1l2;
  line2 += intl2;

  // Print the text to the LCD display
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

/**
*  Display on the screen that system is busy
*/
void printBusy() {
  lcd.clear();
  delay(100);
  // Print the text to the LCD display
  lcd.setCursor(0, 0);
  lcd.print("System is Busy...");
  lcd.setCursor(0, 1);
  lcd.print("Please wait.");
}
#define BTN_DOWN 18
#define BTN_UP 19
#define BTN_ENTER 20

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

void emergencyStop(){
  soilWatering.forceStop();
}