#include "HomeScreen.h"

#include "../Screen.h"

HomeScreen::HomeScreen(Controls *keyboard, Screen *screen,
                       NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
}

void HomeScreen::refresh() {
    const char *menuItems[] = {"Recorder", "Live", nullptr};
    _screen->drawItemList(0, 20, menuItems, _selectedIndex);
}

void HomeScreen::handleEvent(Controls::ButtonEvent event) {
    const char *menuItems[] = {"Recorder", "Live", nullptr};

    int itemCount = 0;
    while (menuItems[itemCount] != nullptr) itemCount++;

    if (event.buttonId == 0) {
        _selectedIndex += event.encoderValue;
        _selectedIndex = constrain(_selectedIndex, 0, itemCount - 1);
        _needsDisplayUpdate = true;
        updateDisplayIfNeeded();
    } else if (event.buttonId == 2 &&
               event.state == PRESSED) {  // Button 1 - Select
        if (_navCallback) {
            AppContext targetContext =
                (_selectedIndex == 0) ? AppContext::RECORDER : AppContext::LIVE;
            _navCallback(targetContext);
            return;
        }
    } else if (event.buttonId == 1 || event.buttonId == 2 || event.buttonId == 3) {
        // Button press - update immediately
        refresh();
    }
}

void HomeScreen::updateDisplayIfNeeded() {
    unsigned long currentTime = millis();

    // Check if we need to update and enough time has passed
    if (_needsDisplayUpdate &&
        (currentTime - _lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS)) {
        refresh();
        _lastDisplayUpdate = currentTime;
        _needsDisplayUpdate = false;
    }
}