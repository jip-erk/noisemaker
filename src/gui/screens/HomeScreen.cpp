#include "HomeScreen.h"

#include "../Screen.h"

HomeScreen::HomeScreen() {};

HomeScreen::HomeScreen(Controls *keyboard, Screen *screen) {
    _keyboard = keyboard;
    _screen = screen;
}

void HomeScreen::handleEvent(Controls::ButtonEvent event) {
    const char *menuItems[] = {"Recorder", "Live", nullptr};

    int itemCount = 0;
    while (menuItems[itemCount] != nullptr) itemCount++;

    static int selectedIndex = 0;

    if (event.buttonId == 0) {
        selectedIndex += event.encoderValue;
        selectedIndex = constrain(selectedIndex, 0, itemCount - 1);
    }

    _screen->drawItemList(0, 20, menuItems, selectedIndex);
}