#include "LiveScreen.h"

LiveScreen::LiveScreen() {
    _screen = nullptr;
}

LiveScreen::LiveScreen(Screen *screen) {
    _screen = screen;
}

LiveScreen::~LiveScreen() {
    // Cleanup if needed
}

void LiveScreen::refresh() {
    // Called when entering Live context
    // Will be drawn by drawSlotView with actual slot data from context
}

void LiveScreen::drawSlotView(const SampleSlot *slots, int selectedSlotIndex, int numSlots, const char **slotLabels) {
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "LIVE - Slots");
    _screen->setNormalFont();

    // Draw slots
    int yPos = 20;
    for (int i = 0; i < numSlots; i++) {
        // Highlight selected slot
        if (i == selectedSlotIndex) {
            _screen->drawBox(0, yPos - 2, 128, 12);
            _screen->getDisplay()->setDrawColor(0);  // Invert text
        }

        // Slot label and sample name
        String slotInfo = String(slotLabels[i]) + ":";
        if (slots[i].isAssigned) {
            // Show sample name
            String displayName = slots[i].sampleName;
            if (displayName.length() > 10) {
                displayName = displayName.substring(0, 7) + "...";
            }
            slotInfo += displayName;
        } else {
            slotInfo += "<empty>";
        }

        _screen->drawStr(2, yPos + 8, slotInfo.c_str());

        if (i == selectedSlotIndex) {
            _screen->getDisplay()->setDrawColor(1);  // Reset
        }

        yPos += 12;
    }

    _screen->display();
}

void LiveScreen::drawSampleSelect(const String *fileList, int selectedFileIndex, int fileCount) {
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "Select Sample");
    _screen->setNormalFont();

    if (fileCount == 0) {
        _screen->drawStr(0, 25, "No samples found");
        _screen->drawStr(0, 40, "Record some first!");
        _screen->display();
        return;
    }

    // Show scrollable file list
    int startIndex = max(0, selectedFileIndex - 2);
    int endIndex = min(fileCount, startIndex + 3);

    int yPos = 20;
    for (int i = startIndex; i < endIndex; i++) {
        if (i == selectedFileIndex) {
            _screen->drawBox(0, yPos - 2, 128, 12);
            _screen->getDisplay()->setDrawColor(0);
        }

        String displayName = fileList[i];
        // Remove extension
        int dotIndex = displayName.lastIndexOf('.');
        if (dotIndex > 0) {
            displayName = displayName.substring(0, dotIndex);
        }

        if (displayName.length() > 15) {
            displayName = displayName.substring(0, 12) + "...";
        }

        _screen->drawStr(2, yPos + 8, displayName.c_str());

        if (i == selectedFileIndex) {
            _screen->getDisplay()->setDrawColor(1);
        }

        yPos += 12;
    }

    // Help text
    _screen->drawStr(0, 60, "B2:OK B3:Clear");

    _screen->display();
}
