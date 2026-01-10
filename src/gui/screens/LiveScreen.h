#ifndef LiveScreen_h
#define LiveScreen_h

#include <Arduino.h>

#include "../../helper/SampleSlot.hpp"
#include "../Screen.h"

class LiveScreen {
   public:
    LiveScreen();
    LiveScreen(Screen *screen);
    ~LiveScreen();

    // UI Display methods
    void refresh();
    void drawSlotView(const SampleSlot *slots, int selectedSlotIndex, int numSlots, const char **slotLabels);
    void drawSampleSelect(const String *fileList, int selectedFileIndex, int fileCount);
    void drawSequencer(const bool sequencerGrid[8][16], int selectedRow, int selectedCol);

   private:
    Screen *_screen;
};

#endif
