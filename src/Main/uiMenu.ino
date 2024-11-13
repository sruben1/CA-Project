void printLcdText(const String& line1, const String& line2) {
    lcd.clear();
    delay(100);
    // Split the text into two lines, with a maximum of 16 characters per line
    int lineLength = 16;
    //String line1 = text.substring(0, min(lineLength, lin1.length()));
    //String line2 = text.substring(min(lineLength, text.length()));

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
    // Print the text to the LCD display
    lcd.setCursor(0, 0);
    lcd.print("System is Busy...");
    lcd.setCursor(0, 1);
    lcd.print("Please wait.");
}