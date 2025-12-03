
#ifndef HomeScreen_h
#define HomeScreen_h

#include <Arduino.h>

#include "../Screen.h"

class HomeScreen {
   public:
    HomeScreen();
    HomeScreen(Screen *screen);
    ~HomeScreen();

    // UI Display methods
    void refresh();
    void drawMenu(const char **menuItems, int selectedIndex, int itemCount);

   private:
    Screen *_screen;
};

#endif
