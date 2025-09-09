
#ifndef HomeScreen_h
#define HomeScreen_h

#include <Arduino.h>

#include "../../hardware/Controls.h"
#include "../Screen.h"

class HomeScreen {
   public:
    HomeScreen();
    HomeScreen(Controls *keyboard, Screen *screen);
    void handleEvent(Controls::ButtonEvent);
    long receiveTimerTick();
    boolean passEventsToMe();
    void showSupporterScreen();
    void showGeneralInformation();
    void showSongSelector();

   private:
    enum Components { NONE = 0, SUPPORTER = 1, SONGSELECTOR = 2 };

    Controls *_keyboard;
    Screen *_screen;

    Components _activeComponent = NONE;
};

#endif
