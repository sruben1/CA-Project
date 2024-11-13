#include <LiquidCrystal.h>

// Define Pins used for LCD Display
LiquidCrystal lcd(12, 11, 9, 8, 7, 6);
//Menu navigation buttons
const int btnRngStart = 2;
const int btnRngEnd = 4;
const int btnLeft = 2;
const int btnRight = 3;
const int btnEnter = 4;
//Main menu page selctor:
bool notInSubMenu = true;
int menuPage = 0;
const unsigned int menuPageCount = 5;
//Sub menu implementation:
int subPageValue = 0;
int currSubPageCount = 1;

void setup() {
  //Debug 
  pinMode(LED_BUILTIN, OUTPUT);
  //UI Menu:
    //LCD:
    lcd.begin(16, 2);
    //Buttons:
    /*for (int i = btnRngStart; i <= btnRngEnd; i++){
      pinMode(i, INPUT);
    }*/
    pinMode(btnLeft, INPUT);
    pinMode(btnRight, INPUT);
    pinMode(btnEnter, INPUT);
}

void loop() {
  //Menu Buttons
  int activeMenuBtn = NULL;
  for (int i = btnRngStart; i <= btnRngEnd; i++){
      if(digitalRead(i) == HIGH){
        activeMenuBtn = i;
      }
    }
  switch(activeMenuBtn){
    case btnLeft:
      //debugLed();
      if(notInSubMenu){
        menuPage = unsignedModulo(menuPage - 1, menuPageCount);
        printLcdText("On Page:" + String(menuPage) ,"");
      } else{
        subPageValue = unsignedModulo(subPageValue - 1, currSubPageCount);
         printLcdText("Page:" + String(menuPage),"On Sub Value:" + String(subPageValue));
      }
      delay(500);
    break;
    case btnRight:
      //debugLed();
      if(notInSubMenu){
        menuPage = unsignedModulo(menuPage + 1, menuPageCount);
        printLcdText("On Page:" + String(menuPage), "");
      } else{
        subPageValue = unsignedModulo(subPageValue + 1, currSubPageCount);
        printLcdText("Page:" + String(menuPage),"On Sub Value:" + String(subPageValue));
      }
      delay(500);
    break;
    case btnEnter:
      //debugLed();
      if(notInSubMenu){
        printLcdText("Entered:"+ String(menuPage),"");
        //TODO : Set the specific range 
        currSubPageCount = 5;
        notInSubMenu = false;
      } else {
        printLcdText("Exited:" + String(menuPage),"");
        notInSubMenu = true;

      }
      delay(500);
    break;
  }
  //add delay to avoid too fast reinterpretation of inputs
}
