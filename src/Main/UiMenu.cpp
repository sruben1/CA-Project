#include "UiMenu.h"

void UiMenu::begin(SimpleLogger& logger, int* prefrences, uint8_t currMenuPageCount, void (*printLcdText)(const String& line1, const String& line2), void (*storePreferences)()) {
  // Store logger reference
  this->logger = &logger;
  
  // Store preferences pointer
  this->preferences = prefrences;

  this->storePreferences = storePreferences;
  
  // Store LCD print function pointer
  this->printLcdText = printLcdText;
  
  // Validate menu page count
  if (currMenuPageCount != MENU_PAGE_COUNT) {
      logger.c("Error: Internal menu page count mismatch");
      return;
  }
  
  // Initialize menu state
  notInSubMenu = true;
  currMenuPage = 0;
  
  // Log initialization
  logger.i("UI Menu initialized");
  
  // Display initial menu page
  if (printLcdText != nullptr) {
      String line1 = mainPageNames[currMenuPage];
      String line2 = String(preferences[currMenuPage]);
      printLcdText(line1, line2);
  } else {
      logger.c("Error: LCD print function is null");
  }
}

void UiMenu::handleButtonUp(){
  if (notInSubMenu) {
    currMenuPage = unsignedModulo(currMenuPage + 1, MENU_PAGE_COUNT);
    printLcdText(mainPageNames[currMenuPage], "");
  } else {
    subPageValue = unsignedModulo(subPageValue - subPageStepSize[currMenuPage], subPageValuesRange[currMenuPage*2+1]);
    printLcdText(mainPageNames[currMenuPage], String(subPageValue));
  }
  logUnsignedDebug("Handled BUTTON UP, now menu: %u, sub menu: %u", currMenuPage, subPageValue);
  delay(BUTTON_DEBOUNCE_DELAY);
}

void UiMenu::handleButtonDown(){
  if (notInSubMenu) {
    currMenuPage = unsignedModulo(currMenuPage - 1, MENU_PAGE_COUNT);
    printLcdText(mainPageNames[currMenuPage], "");
  } else {
    subPageValue = unsignedModulo(subPageValue + subPageStepSize[currMenuPage], subPageValuesRange[currMenuPage*2+1]);
    printLcdText(mainPageNames[currMenuPage], String(subPageValue));
  }
  logUnsignedDebug("Handled BUTTON DOWN, now menu: %u, sub menu: %u", currMenuPage, subPageValue);
  delay(BUTTON_DEBOUNCE_DELAY);
}

void UiMenu::handleButtonEnter() {
    logUnsignedDebug("Handling ENTER, for menu: %u, sub menu: %u", currMenuPage, subPageValue);
    if (notInSubMenu) {
      logger->d("ENTER: ENTERING sub menu.");
        printLcdText(mainPageNames[currMenuPage], "Edit: + or -");
        subPageValue = preferences[currMenuPage];
        notInSubMenu = false;
    } else {
      logger->d("ENTER: EXITING sub menu.");
        if (currMenuPage == 12 && subPageValue == 1) {
            if (storePreferences != nullptr) {
                logger->i("Storing settings due to ENTER action:");
                storePreferences();
                printLcdText("To SD:", mainPageNames[currMenuPage]);
            } else {
                logger->c("Error: storePreferences is null");
            }
        } else if (currMenuPage == 14) {
            //TODO
        } else {
            logUnsignedDebug("ENTER: storing to cache sub menu value: %u, for menu: %u", subPageValue, currMenuPage);
            preferences[currMenuPage] = subPageValue;
            printLcdText("Saved:", mainPageNames[currMenuPage]);
            notInSubMenu = true;
        }
    }
    delay(BUTTON_DEBOUNCE_DELAY);
}

unsigned UiMenu::unsignedModulo(int value, unsigned int m) {
  int mod = value % (int)m;
  if (mod < 0) {
    mod += m;
  }
  return mod;
}

void UiMenu::logUnsignedDebug(const char* format, const unsigned value){
  static char buffer[128];
  snprintf(buffer, sizeof(buffer), format, value);
  logger->d(buffer);
}

void UiMenu::logUnsignedDebug(const char* format, const unsigned value1, const unsigned value2){
  static char buffer[128];
  snprintf(buffer, sizeof(buffer), format, value1, value2);
  logger->d(buffer);
}
