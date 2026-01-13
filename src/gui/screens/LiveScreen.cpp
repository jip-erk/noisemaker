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

void LiveScreen::drawSequencer(const bool sequencerGrid[][16], int selectedTrack,
                               int currentStep, int currentBPM, bool isPlaying,
                               int numTracks, int numSteps) {
    _screen->clear();
    _screen->setHeaderFont();

    // Header with BPM and play status
    char header[20];
    sprintf(header, "SEQ %dBPM %s", currentBPM, isPlaying ? "[>]" : "[ ]");
    _screen->drawStr(0, 10, header);
    _screen->setNormalFont();

    const int trackLabelWidth = 16;  // Space for track labels (e.g., "kick")
    const int cellWidth = 7;         // Width of each cell
    const int cellHeight = 12;       // Height of each cell (increased for 4 tracks)
    const int gridStartX = trackLabelWidth + 1;
    const int gridStartY = 14;

    // Draw track labels and grid cells
    for (int track = 0; track < numTracks; track++) {
        int yPos = gridStartY + (track * cellHeight);

        // Draw track label (or placeholder)
        const char* trackLabels[] = {"kick", "snare", "hat", "perc"};
        if (track < 4) {
            _screen->drawStr(0, yPos + 9, trackLabels[track]);
        }

        // Draw 16 steps for this track
        for (int step = 0; step < numSteps; step++) {
            int xPos = gridStartX + (step * cellWidth);

            bool isCurrentStep = (step == currentStep && isPlaying);
            bool isSelected = (track == selectedTrack && step == currentStep);
            bool isActive = sequencerGrid[track][step];

            // Draw cell based on state
            if (isCurrentStep) {
                // Playhead indicator - thick border
                _screen->drawBox(xPos - 1, yPos - 1, cellWidth + 2, cellHeight + 2);
                if (isActive) {
                    _screen->drawBox(xPos + 1, yPos + 1, cellWidth - 2, cellHeight - 2);
                }
            } else if (isSelected) {
                // Selected cell - inverted
                _screen->drawBox(xPos, yPos, cellWidth, cellHeight);
                _screen->getDisplay()->setDrawColor(0);  // Invert
                if (isActive) {
                    _screen->drawBox(xPos + 2, yPos + 2, cellWidth - 4, cellHeight - 4);
                }
                _screen->getDisplay()->setDrawColor(1);  // Reset
            } else {
                // Normal cell
                if (isActive) {
                    _screen->drawBox(xPos + 1, yPos + 1, cellWidth - 2, cellHeight - 2);
                } else {
                    // Draw thin border using U8G2 drawFrame
                    _screen->getDisplay()->drawFrame(xPos, yPos, cellWidth, cellHeight);
                }
            }
        }
    }

    // Footer help text
    _screen->drawStr(0, 63, "Enc:Trk B2:Tog B3:Play");

    _screen->display();
}
