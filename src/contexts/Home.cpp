#include "Home.h"

// Menu items for the home screen
const char* Home::MENU_ITEMS[NUM_MENU_ITEMS + 1] = {"Recorder", "Live", "Files",
                                                    nullptr};

Home::Home(Controls* keyboard, Screen* screen, NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
    _selectedIndex = 0;
    currentState = HOME_MENU;
    _homeScreen = HomeScreen(screen);
}

void Home::refresh() {
    currentState = HOME_MENU;
    _selectedIndex = 0;
    drawMenu();
}

void Home::drawMenu() {
    _homeScreen.drawMenu(MENU_ITEMS, _selectedIndex, NUM_MENU_ITEMS);
}

void Home::handleEvent(Controls::ButtonEvent event) {
    // Button 5 - Up navigation
    if (event.buttonId == 5 && event.state == PRESSED) {
        _selectedIndex--;
        _selectedIndex = constrain(_selectedIndex, 0, NUM_MENU_ITEMS - 1);
        drawMenu();
        return;
    }

    // Button 3 - Down navigation
    if (event.buttonId == 3 && event.state == PRESSED) {
        _selectedIndex++;
        _selectedIndex = constrain(_selectedIndex, 0, NUM_MENU_ITEMS - 1);
        drawMenu();
        return;
    }

    // Button 4 - Select
    if (event.buttonId == 4 && event.state == PRESSED) {
        if (_navCallback) {
            AppContext targetContext;
            if (_selectedIndex == 0)
                targetContext = AppContext::RECORDER;
            else if (_selectedIndex == 1)
                targetContext = AppContext::LIVE;
            else
                targetContext = AppContext::SAMPLE_MANAGER;

            _navCallback(targetContext);
            return;
        }
    }
}
