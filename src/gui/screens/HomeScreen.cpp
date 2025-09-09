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
    } else if (event.buttonId == 2 &&
               event.state == PRESSED) {  // Button 1 - Select
        if (_navCallback) {
            AppContext targetContext =
                (_selectedIndex == 0) ? AppContext::RECORDER : AppContext::LIVE;
            _navCallback(targetContext);
            return;
        }
    }

    refresh();
}