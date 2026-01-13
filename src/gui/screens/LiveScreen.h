#ifndef LiveScreen_h
#define LiveScreen_h

#include <Arduino.h>

#include "../../helper/Track.hpp"
#include "../Screen.h"

class LiveScreen {
   public:
    LiveScreen();
    LiveScreen(Screen *screen);
    ~LiveScreen();

    // UI Display methods
    void refresh();
    void drawTrackView(const Track *tracks, int selectedTrackIndex, int numTracks, const char **trackLabels);
    void drawSampleSelect(const String *fileList, int selectedFileIndex, int fileCount);
    void drawSequencer(const bool sequencerGrid[][16], int selectedTrack, int currentStep, int currentBPM, bool isPlaying, int numTracks, int numSteps);

   private:
    Screen *_screen;
};

#endif
