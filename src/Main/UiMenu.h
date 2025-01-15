#ifndef UI_MENU_H
#define UI_MENU_H

#include <Arduino.h>
#include <SimpleLogger.h>

// Total number of menupages:
#define MENU_PAGE_COUNT 14

#define BUTTON_DEBOUNCE_DELAY 500

class UiMenu {
public:
  void begin(SimpleLogger& logger, int* prefrences, uint8_t menuPageCount, void (*printLcdText)(const String& line1, const String& line2), void (*storePreferences)());
  void handleButtonUp();
  void handleButtonDown();
  void handleButtonEnter();



private:
  const char* mainPageNames[MENU_PAGE_COUNT] = { "S. read intv.:", "Th. soil node1:", "Th. soil node2:", "Th. soil node3:", "Th. soil node4:",
                                                 "Th. soil node5:", "Th. soil node6:", "Th. soil node7:", "Th. soil node8:", "Th. soil node9:", "°C thresh:", "Hum thresh:",
                                                 "Save pref:", "Prep shut down:" };
  // Sub menu implementation:
  int subPageValue = 0;
  int currLowerLimit = -1;
  int currUpperLimit = 1;
  const int subPageValuesRange[MENU_PAGE_COUNT*2] = { 20, 3600, 0, 11, 0, 11, 0, 11, 0, 11, 0, 11, 0, 11, 0, 11, 0, 11, 0, 11, 17, 45, 17, 45, 0, 1, 0, 1 };
  const int subPageStepSize[MENU_PAGE_COUNT] = { 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const int subPageNameAssignment[MENU_PAGE_COUNT]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 }; // Which naming scheme is used for the subpage variables, 0 := Bare integer values, 1 := Boolenan (1=true, 0=false)

  int* preferences;
  void (*storePreferences)();

  void (*printLcdText)(const String& line1, const String& line2);

  bool notInSubMenu = true;
  int currMenuPage = 0;

  SimpleLogger* logger = nullptr;
  void logUnsignedDebug(const char* format, const unsigned value);
  void logUnsignedDebug(const char* format, const unsigned value1, const unsigned value2);

  unsigned int unsignedModulo(int value, unsigned int m);

};

#endif