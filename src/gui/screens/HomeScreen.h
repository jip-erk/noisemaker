
#ifndef HomeScreen_h
#define HomeScreen_h

#include <Arduino.h>

#include "../../hardware/Controls.h"
#include "../../main.h"
#include "../Screen.h"

class HomeScreen {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);

    HomeScreen(Controls *keyboard, Screen *screen,
               NavigationCallback navCallback = nullptr);

    void handleEvent(Controls::ButtonEvent);
    void refresh();
    
   private:
    NavigationCallback _navCallback;

    enum Components { NONE = 0, SUPPORTER = 1, SONGSELECTOR = 2 };

    Controls *_keyboard;
    Screen *_screen;
    int _selectedIndex = 0;
    Components _activeComponent = NONE;
};

#endif
