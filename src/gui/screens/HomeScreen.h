
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
    Controls *_keyboard;
    Screen *_screen;
    int _selectedIndex = 0;

    // Rate limiting for display updates
    unsigned long _lastDisplayUpdate = 0;
    bool _needsDisplayUpdate = false;
    static const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 50;  // 20 Hz max

    void updateDisplayIfNeeded();
};

#endif
