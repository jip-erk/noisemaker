#include "LiveScreen.h"

// Default MIDI note mappings (C3=60, C#3=61, D3=62, D#3=63)
const uint8_t LiveScreen::DEFAULT_MIDI_NOTES[NUM_SLOTS] = {60, 61, 62, 63};

// Slot labels for display
const char* LiveScreen::SLOT_LABELS[NUM_SLOTS] = {"kick", "snare", "hat", "perc"};

LiveScreen::LiveScreen(Controls *keyboard, Screen *screen,
                       NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
    _audioResources = nullptr;

    // Initialize slots with default MIDI notes
    for (int i = 0; i < NUM_SLOTS; i++) {
        _slots[i].midiNote = DEFAULT_MIDI_NOTES[i];
    }
}

LiveScreen::~LiveScreen() {
    // Cleanup if needed
}

void LiveScreen::refresh() {
    currentState = LIVE_SLOT_VIEW;
    _selectedSlotIndex = 0;
    drawSlotView();
}

void LiveScreen::setAudioResources(AudioResources* audioResources) {
    _audioResources = audioResources;
}

void LiveScreen::handleMidiNote(uint8_t note, uint8_t velocity) {
    // Only handle note-on events (velocity > 0)
    if (velocity == 0) return;

    // Find the slot that matches this MIDI note
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (_slots[i].midiNote == note && _slots[i].isAssigned) {
            playSlot(i);

            Serial.print("MIDI trigger slot ");
            Serial.print(i);
            Serial.print(" (");
            Serial.print(SLOT_LABELS[i]);
            Serial.print(") with note ");
            Serial.println(note);
            break;
        }
    }
}

void LiveScreen::handleEvent(Controls::ButtonEvent event) {
    // Back button (button 1) - depends on state
    if (event.buttonId == 1 && event.state == PRESSED) {
        if (currentState == LIVE_SLOT_VIEW) {
            // Return to home
            if (_navCallback) {
                _navCallback(AppContext::HOME);
                return;
            }
        } else if (currentState == LIVE_SAMPLE_SELECT) {
            // Return to slot view
            currentState = LIVE_SLOT_VIEW;
            drawSlotView();
        }
        return;
    }

    // Button 2 - Select/Confirm
    if (event.buttonId == 2 && event.state == PRESSED) {
        if (currentState == LIVE_SLOT_VIEW) {
            // Enter sample selection for this slot
            loadFileList();
            currentState = LIVE_SAMPLE_SELECT;
            _selectedFileIndex = 0;
            drawSampleSelect();
        } else if (currentState == LIVE_SAMPLE_SELECT) {
            // Assign selected sample to slot
            assignSampleToSlot();
            currentState = LIVE_SLOT_VIEW;
            drawSlotView();
        }
        return;
    }

    // Button 3 - Play/Stop slot
    if (event.buttonId == 3 && event.state == PRESSED) {
        if (currentState == LIVE_SLOT_VIEW) {
            // Play the selected slot
            if (_slots[_selectedSlotIndex].isAssigned) {
                playSlot(_selectedSlotIndex);
            }
        } else if (currentState == LIVE_SAMPLE_SELECT) {
            // Clear the slot
            clearSlot(_selectedSlotIndex);
            currentState = LIVE_SLOT_VIEW;
            drawSlotView();
        }
        return;
    }

    // Encoder - Navigation
    if (event.buttonId == 0 && event.encoderValue != 0) {
        if (currentState == LIVE_SLOT_VIEW) {
            _selectedSlotIndex += event.encoderValue;
            _selectedSlotIndex = constrain(_selectedSlotIndex, 0, NUM_SLOTS - 1);
            drawSlotView();
        } else if (currentState == LIVE_SAMPLE_SELECT) {
            _selectedFileIndex += event.encoderValue;
            _selectedFileIndex = constrain(_selectedFileIndex, 0, max(0, _fileCount - 1));
            drawSampleSelect();
        }
        return;
    }
}

void LiveScreen::drawSlotView() {
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "LIVE - Slots");
    _screen->setNormalFont();

    // Draw 4 slots
    int yPos = 20;
    for (int i = 0; i < NUM_SLOTS; i++) {
        // Highlight selected slot
        if (i == _selectedSlotIndex) {
            _screen->drawBox(0, yPos - 2, 128, 12);
            _screen->getDisplay()->setDrawColor(0);  // Invert text
        }

        // Slot label and sample name
        String slotInfo = String(SLOT_LABELS[i]) + ":";
        if (_slots[i].isAssigned) {
            // Show sample name
            String displayName = _slots[i].sampleName;
            if (displayName.length() > 10) {
                displayName = displayName.substring(0, 7) + "...";
            }
            slotInfo += displayName;
        } else {
            slotInfo += "<empty>";
        }

        _screen->drawStr(2, yPos + 8, slotInfo.c_str());

        if (i == _selectedSlotIndex) {
            _screen->getDisplay()->setDrawColor(1);  // Reset
        }

        yPos += 12;
    }

    _screen->display();
}

void LiveScreen::drawSampleSelect() {
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "Select Sample");
    _screen->setNormalFont();

    if (_fileCount == 0) {
        _screen->drawStr(0, 25, "No samples found");
        _screen->drawStr(0, 40, "Record some first!");
        _screen->display();
        return;
    }

    // Show scrollable file list
    int startIndex = max(0, _selectedFileIndex - 2);
    int endIndex = min(_fileCount, startIndex + 3);

    int yPos = 20;
    for (int i = startIndex; i < endIndex; i++) {
        if (i == _selectedFileIndex) {
            _screen->drawBox(0, yPos - 2, 128, 12);
            _screen->getDisplay()->setDrawColor(0);
        }

        String displayName = getFileNameWithoutExtension(_fileList[i]);
        if (displayName.length() > 15) {
            displayName = displayName.substring(0, 12) + "...";
        }

        _screen->drawStr(2, yPos + 8, displayName.c_str());

        if (i == _selectedFileIndex) {
            _screen->getDisplay()->setDrawColor(1);
        }

        yPos += 12;
    }

    // Help text
    _screen->drawStr(0, 60, "B2:OK B3:Clear");

    _screen->display();
}

void LiveScreen::loadFileList() {
    _fileCount = 0;

    if (!SD.exists("/RECORDINGS")) {
        Serial.println("RECORDINGS directory does not exist");
        return;
    }

    File recordingsDir = SD.open("/RECORDINGS");
    if (!recordingsDir || !recordingsDir.isDirectory()) {
        Serial.println("Failed to open RECORDINGS directory");
        if (recordingsDir) recordingsDir.close();
        return;
    }

    // Read all .WAV files
    while (true && _fileCount < 20) {
        File entry = recordingsDir.openNextFile();
        if (!entry) break;

        String filename = entry.name();
        if (!entry.isDirectory()) {
            if (filename.endsWith(".WAV") || filename.endsWith(".wav")) {
                // Skip .bdf files
                if (!filename.endsWith(".bdf") && !filename.endsWith(".BDF")) {
                    _fileList[_fileCount] = filename;
                    _fileCount++;
                }
            }
        }
        entry.close();
    }

    recordingsDir.close();
    Serial.println("Loaded " + String(_fileCount) + " samples");
}

void LiveScreen::assignSampleToSlot() {
    if (_selectedFileIndex >= _fileCount) return;

    String fileName = getFileNameWithoutExtension(_fileList[_selectedFileIndex]);
    _slots[_selectedSlotIndex].assignSample(fileName, DEFAULT_MIDI_NOTES[_selectedSlotIndex]);

    Serial.print("Assigned '");
    Serial.print(fileName);
    Serial.print("' to slot ");
    Serial.println(_selectedSlotIndex);
}

void LiveScreen::clearSlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= NUM_SLOTS) return;

    stopSlot(slotIndex);
    _slots[slotIndex].clear();
    _slots[slotIndex].midiNote = DEFAULT_MIDI_NOTES[slotIndex];

    Serial.print("Cleared slot ");
    Serial.println(slotIndex);
}

void LiveScreen::playSlot(int slotIndex) {
    if (!_audioResources || slotIndex < 0 || slotIndex >= NUM_SLOTS) return;
    if (!_slots[slotIndex].isAssigned) return;

    // Get the appropriate WAV player for this slot
    AudioPlaySdWavExtended* player = nullptr;
    switch (slotIndex) {
        case 0: player = &_audioResources->playWav1; break;
        case 1: player = &_audioResources->playWav2; break;
        case 2: player = &_audioResources->playWav3; break;
        case 3: player = &_audioResources->playWav4; break;
    }

    if (player) {
        String wavPath = _slots[slotIndex].getWavPath();
        uint32_t startByte = _slots[slotIndex].getStartByte();
        uint32_t endByte = _slots[slotIndex].getEndByte();

        Serial.print("Playing slot ");
        Serial.print(slotIndex);
        Serial.print(": ");
        Serial.print(wavPath);
        Serial.print(" [");
        Serial.print(startByte);
        Serial.print(" - ");
        Serial.print(endByte);
        Serial.println("]");

        player->play(wavPath.c_str(), startByte, endByte, 1.0);
    }
}

void LiveScreen::stopSlot(int slotIndex) {
    if (!_audioResources || slotIndex < 0 || slotIndex >= NUM_SLOTS) return;

    AudioPlaySdWavExtended* player = nullptr;
    switch (slotIndex) {
        case 0: player = &_audioResources->playWav1; break;
        case 1: player = &_audioResources->playWav2; break;
        case 2: player = &_audioResources->playWav3; break;
        case 3: player = &_audioResources->playWav4; break;
    }

    if (player && player->isPlaying()) {
        player->stop();
    }
}

void LiveScreen::stopAllSlots() {
    for (int i = 0; i < NUM_SLOTS; i++) {
        stopSlot(i);
    }
}

String LiveScreen::getFileNameWithoutExtension(const String& fileName) {
    int dotIndex = fileName.lastIndexOf('.');
    if (dotIndex > 0) {
        return fileName.substring(0, dotIndex);
    }
    return fileName;
}
