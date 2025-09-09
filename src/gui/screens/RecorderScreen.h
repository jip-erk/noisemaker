#ifndef RecorderScreen_h
#define RecorderScreen_h

#include <Arduino.h>

#include "../../helper/AudioResources.h"
#include "../Screen.h"

class RecorderScreen {
   public:
    RecorderScreen();
    RecorderScreen(Screen *screen, AudioResources *audioResources);

    void showRecorderScreen(boolean onScreen);
    void showRecorderScreenRecording(boolean onScreen);
    void drawInputPeakMeter(float peak, boolean onScreen);

   private:
    Screen *_screen;
    AudioResources *_audioResources;
    Screen::Area _peakArea;
    Screen::Area _textArea;
};

#endif
