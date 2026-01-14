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

void LiveScreen::drawMainView(const bool sequencerGrid[][32], int selectedTrack,
                              int currentStep, int currentBPM, bool isPlaying,
                              int numTracks, int numSteps, int currentPage,
                              int stepRange, const char** trackLabels,
                              const Track* tracks, float currentVolume,
                              float currentPitch) {
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
    char trackStr[12];
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

    // ===== Visualizers (Volume & Pitch) =====
    // Positioned side-by-side below the sample name
    // Center Y for knobs = 42

    int knobY = 35;
    int knobRadius = 8;  // Small radius
    int textYOffset = knobRadius + 10;

    // --- Volume Knob (Left) ---
    int volX = 11;
    // Center leftish
    _screen->getDisplay()->drawCircle(volX, knobY, knobRadius);

    // Vol Value: 0.0 to 4.0
    // Draw 1.0 exactly at center (top/12 o'clock)
    float volNorm;
    float vVal = constrain(currentVolume, 0.0f, 4.0f);

    if (vVal <= 1.0f) {
        // Map 0.0..1.0 -> 0.0..0.5
        volNorm = (vVal / 1.0f) * 0.5f;
    } else {
        // Map 1.0..4.0 -> 0.5..1.0
        volNorm = 0.5f + ((vVal - 1.0f) / 3.0f) * 0.5f;
    }

    float startAngle = 135.0f * (PI / 180.0f);
    float endAngle = 405.0f * (PI / 180.0f);
    float volAngle = startAngle + (volNorm * (endAngle - startAngle));

    int lineLen = knobRadius - 2;
    _screen->getDisplay()->drawLine(volX, knobY, volX + cos(volAngle) * lineLen,
                                    knobY + sin(volAngle) * lineLen);

    // Label "Vol"
    _screen->setNormalFont();
    char volStr[10];
    sprintf(volStr, "V:%d%%", (int)(currentVolume * 100));
    int vW = _screen->getDisplay()->getStrWidth(volStr);
    _screen->drawStr(volX - (vW / 2), knobY + textYOffset, volStr);

    // --- Pitch Knob (Right) ---
    int pitchX = 37;  // Center rightish
    _screen->getDisplay()->drawCircle(pitchX, knobY, knobRadius);

    // Pitch Value: 0.1 to 2.0
    // Draw 1.0 exactly at center (top/12 o'clock)
    float pitchNorm;
    float pVal = constrain(currentPitch, 0.1f, 2.0f);

    if (pVal <= 1.0f) {
        // Map 0.1..1.0 -> 0.0..0.5
        pitchNorm = ((pVal - 0.1f) / 0.9f) * 0.5f;
    } else {
        // Map 1.0..2.0 -> 0.5..1.0
        pitchNorm = 0.5f + ((pVal - 1.0f) / 1.0f) * 0.5f;
    }

    float pitchAngle = startAngle + (pitchNorm * (endAngle - startAngle));
    _screen->getDisplay()->drawLine(pitchX, knobY,
                                    pitchX + cos(pitchAngle) * lineLen,
                                    knobY + sin(pitchAngle) * lineLen);

    char pitchStr[10];
    int pInt = (int)currentPitch;
    int pFrac = (int)((currentPitch - pInt) * 100);
    sprintf(pitchStr, "P:%d.%02d", pInt, pFrac);
    int pW = _screen->getDisplay()->getStrWidth(pitchStr);
    _screen->drawStr(pitchX - (pW / 2), knobY + textYOffset, pitchStr);

    // Standard footer
    char footer[30];
    int displayRangeStart = stepRange + 1;
    int displayRangeEnd = stepRange + 16;
    sprintf(footer, "Pg %d/8 (%d-%d)", currentPage + 1, displayRangeStart,
            displayRangeEnd);
    _screen->drawStr(0, 63, footer);

    _screen->display();
}
