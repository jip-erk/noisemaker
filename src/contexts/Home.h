#ifndef Home_h
#define Home_h

#include <Arduino.h>

#include "../hardware/Controls.h"
#include "../main.h"
#include "../gui/Screen.h"
#include "../gui/screens/HomeScreen.h"

class Home {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);

    Home(Controls *keyboard, Screen *screen, NavigationCallback navCallback = nullptr);

    void refresh();
    void handleEvent(Controls::ButtonEvent);

    enum HomeState {
        HOME_MENU = 0  // Main menu view
    };

    HomeState currentState = HOME_MENU;

   private:
    NavigationCallback _navCallback;
    Controls *_keyboard;
    Screen *_screen;
    HomeScreen _homeScreen;

    // Menu state
    int _selectedIndex = 0;
    static const int NUM_MENU_ITEMS = 2;
    static const char *MENU_ITEMS[NUM_MENU_ITEMS];

    // Private methods
    void drawMenu();
};

#endif
