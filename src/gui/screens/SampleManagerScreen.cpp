#include "SampleManagerScreen.h"

SampleManagerScreen::SampleManagerScreen() { _screen = nullptr; }

SampleManagerScreen::SampleManagerScreen(Screen* screen) { _screen = screen; }

SampleManagerScreen::~SampleManagerScreen() {}

void SampleManagerScreen::drawList(const String* fileList, int selectedIndex,
                                   int fileCount) {
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "File Manager");
    _screen->setNormalFont();

    if (fileCount == 0) {
        _screen->drawStr(0, 30, "No files found");
        _screen->display();
        return;
    }

    // Visible items
    int visibleItems = 3;
    int startIndex = 0;

    if (selectedIndex >= visibleItems) {
        startIndex = selectedIndex - (visibleItems - 1);
    }
    
    int endIndex = min(fileCount, startIndex + visibleItems);
    int yPos = 20;

    for (int i = startIndex; i < endIndex; i++) {
        if (i == selectedIndex) {
            _screen->drawBox(0, yPos - 2, 128, 12);
            _screen->getDisplay()->setDrawColor(0);  // Inverted text
        }

        String displayName = fileList[i];
        
        // Truncate if too long
        if (displayName.length() > 20) {
            displayName = displayName.substring(0, 17) + "...";
        }

        _screen->drawStr(2, yPos + 8, displayName.c_str());

        if (i == selectedIndex) {
            _screen->getDisplay()->setDrawColor(1);  // Reset
        }

        yPos += 12;
    }
    
    // Commands footer
    _screen->drawStr(0, 60, "B2: Back  B4: Del");

    _screen->display();
}
