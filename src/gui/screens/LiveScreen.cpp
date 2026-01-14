#include "LiveScreen.h"

LiveScreen::LiveScreen() { _screen = nullptr; }

LiveScreen::LiveScreen(Screen* screen) { _screen = screen; }

LiveScreen::~LiveScreen() {
    // Cleanup if needed
}

void LiveScreen::refresh() {
    // Called when entering Live context
    // Will be drawn by drawTrackView with actual track data from context
}

void LiveScreen::drawTrackView(const Track* tracks, int selectedTrackIndex,
                               int numTracks, const char** trackLabels) {
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "LIVE - Tracks");
    _screen->setNormalFont();

    // Draw tracks
    int yPos = 20;
    for (int i = 0; i < numTracks; i++) {
        // Highlight selected track
        if (i == selectedTrackIndex) {
            _screen->drawBox(0, yPos - 2, 128, 12);
            _screen->getDisplay()->setDrawColor(0);  // Invert text
        }

        // Track label and sample name
        String trackInfo = String(trackLabels[i]) + ":";
        if (tracks[i].isAssigned) {
            // Show sample name
            String displayName = tracks[i].fileName;
            if (displayName.length() > 10) {
                displayName = displayName.substring(0, 7) + "...";
            }
            trackInfo += displayName;
        } else {
            trackInfo += "<empty>";
        }

        _screen->drawStr(2, yPos + 8, trackInfo.c_str());

        if (i == selectedTrackIndex) {
            _screen->getDisplay()->setDrawColor(1);  // Reset
        }

        yPos += 12;
    }

    _screen->display();
}

void LiveScreen::drawSampleSelect(const String* fileList, int selectedFileIndex,
                                  int fileCount) {
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

void LiveScreen::drawMainView(const bool sequencerGrid[][16], int selectedTrack,
                              int currentStep, int currentBPM, bool isPlaying,
                              int numTracks, int numSteps, int currentPage,
                              const char** trackLabels, const Track* tracks,
                              int activeVolumeTrack, float activeVolume) {
    _screen->clear();

    // ===== Header Layout =====
    const int headerHeight = 10;
    const int trackBoxWidth = 14;
    const int playIconSize = 6;
    const int playIconX = 0;
    const int playIconY = 2;

    if (!isPlaying) {
        // Play icon: triangle pointing right
        _screen->getDisplay()->drawLine(playIconX, playIconY, playIconX,
                                        playIconY + playIconSize);
        _screen->getDisplay()->drawLine(playIconX, playIconY,
                                        playIconX + playIconSize,
                                        playIconY + playIconSize / 2);
        _screen->getDisplay()->drawLine(playIconX, playIconY + playIconSize,
                                        playIconX + playIconSize,
                                        playIconY + playIconSize / 2);
    } else {
        // Pause icon: two vertical bars
        _screen->getDisplay()->drawVLine(playIconX + 1, playIconY,
                                         playIconSize);
        _screen->getDisplay()->drawVLine(playIconX + 4, playIconY,
                                         playIconSize);
    }

    // Track number box after play/pause icon
    const int trackBoxX = 12;
    _screen->drawBox(trackBoxX, 0, trackBoxWidth, headerHeight);

    // Draw track number (01, 02, etc.)
    _screen->setNormalFont();
    char trackStr[3];
    sprintf(trackStr, "%02d", selectedTrack + 1);
    _screen->getDisplay()->setDrawColor(0);  // Invert for white box
    _screen->drawStr(trackBoxX + 4, 7, trackStr);
    _screen->getDisplay()->setDrawColor(1);  // Reset
    _screen->setNormalFont();

    // SEQ label
    _screen->setHeaderFont();
    _screen->drawStr(trackBoxX + trackBoxWidth + 4, 10, "SEQ");
    _screen->setNormalFont();

    // Sample name under header
    String sampleName = "<empty>";
    if (selectedTrack >= 0 && selectedTrack < numTracks && tracks) {
        if (tracks[selectedTrack].isAssigned) {
            sampleName = tracks[selectedTrack].fileName;
            // Truncate if too long for display
            if (sampleName.length() > 18) {
                sampleName = sampleName.substring(0, 15) + "...";
            }
        }
    }
    _screen->drawStr(0, 20, sampleName.c_str());

    // BPM on right side

    // Footer with page info or volume display
    char footer[30];
    if (activeVolumeTrack >= 0 && activeVolumeTrack < 4) {
        int volumePercent = (int)(activeVolume * 100);
        sprintf(footer, "T%d Vol:%d%%", activeVolumeTrack + 1, volumePercent);
    } else {
        sprintf(footer, "Page %d/4", currentPage + 1);
    }
    _screen->drawStr(0, 63, footer);

    _screen->display();
}
