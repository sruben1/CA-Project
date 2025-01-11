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
        logger.w("Error: Menu page count mismatch");
        return;
    }
    
    // Initialize menu state
    notInSubMenu = true;
    currMenuPage = 0;
    
    // Initialize sub-menu values
    subPageValue = 0;
    
    // Set initial limits based on first menu item
    currLowerLimit = subPageValuesRange[0];
    currUpperLimit = subPageValuesRange[1];
    
    // Log initialization
    logger.i("UI Menu initialized");
    
    // Display initial menu page
    if (printLcdText != nullptr) {
        String line1 = mainPageNames[currMenuPage];
        String line2 = String(preferences[currMenuPage]);
        printLcdText(line1, line2);
    } else {
        logger.w("Error: LCD print function is null");
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
  delay(500);
}

void UiMenu::handleButtonDown(){
  if (notInSubMenu) {
    currMenuPage = unsignedModulo(currMenuPage - 1, MENU_PAGE_COUNT);
    printLcdText(mainPageNames[currMenuPage], "");
  } else {
    subPageValue = unsignedModulo(subPageValue + subPageStepSize[currMenuPage], subPageValuesRange[currMenuPage*2+1]);
    printLcdText(mainPageNames[currMenuPage], String(subPageValue));
  }
  delay(500);
}

void UiMenu::handleButtonEnter() {
    if (notInSubMenu) {
        printLcdText(mainPageNames[currMenuPage], "Edit: + or -");
        subPageValue = preferences[currMenuPage];
        notInSubMenu = false;
    } else {
        if (currMenuPage == 13) {  // Changed = to == for comparison
            if (storePreferences != nullptr) {  // Add null check
                storePreferences();
                printLcdText("To SD:", mainPageNames[currMenuPage]);
            } else {
                logger->w("Error: storePreferences is null");
            }
        } else if (currMenuPage == 14) {  // Changed = to == for comparison
            //TODO
        } else {
            preferences[currMenuPage] = subPageValue;
            printLcdText("Saved:", mainPageNames[currMenuPage]);
            notInSubMenu = true;
        }
    }
    delay(500);
}

unsigned UiMenu::unsignedModulo(int value, unsigned int m) {
  int mod = value % (int)m;
  if (mod < 0) {
    mod += m;
  }
  return mod;
}

