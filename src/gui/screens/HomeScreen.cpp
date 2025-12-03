#include "HomeScreen.h"

HomeScreen::HomeScreen() {
    _screen = nullptr;
}

HomeScreen::HomeScreen(Screen *screen) {
    _screen = screen;
}

HomeScreen::~HomeScreen() {
    // Cleanup if needed
}

void HomeScreen::refresh() {
    // Called when entering Home context
    // Will be drawn by drawMenu with actual menu data from context
}

void HomeScreen::drawMenu(const char **menuItems, int selectedIndex, int itemCount) {
    _screen->clear();
    _screen->drawItemList(0, 20, menuItems, selectedIndex);
    _screen->display();
}