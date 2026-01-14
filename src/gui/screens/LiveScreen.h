#ifndef LiveScreen_h
#define LiveScreen_h

#include <Arduino.h>

#include "../../helper/Track.hpp"
#include "../Screen.h"

class LiveScreen {
   public:
    LiveScreen();
    LiveScreen(Screen* screen);
    ~LiveScreen();

    // UI Display methods
    void refresh();
    void drawMainView(const bool sequencerGrid[][16], int selectedTrack,
                      int currentStep, int currentBPM, bool isPlaying,
                      int numTracks, int numSteps, int currentPage,
                      const char** trackLabels, const Track* tracks,
                      int activeVolumeTrack = -1, float activeVolume = 0.5f);
    void drawTrackView(const Track* tracks, int selectedTrackIndex,
                       int numTracks, const char** trackLabels);
    void drawSampleSelect(const String* fileList, int selectedFileIndex,
                          int fileCount);

   private:
    Screen* _screen;
};

#endif
