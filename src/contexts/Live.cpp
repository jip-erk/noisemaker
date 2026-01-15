#include "Live.h"

// Track labels for display
const char* Live::TRACK_LABELS[NUM_TRACKS] = {"A", "B", "C", "D",
                                              "E", "F", "G"};

Live::Live(Controls* keyboard, Screen* screen, NavigationCallback navCallback,
           TimerResetCallback timerCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
    _timerCallback = timerCallback;
    _audioResources = nullptr;
    _liveScreen = LiveScreen(screen);

    // _sequencer initialized by its constructor

    // Initialize track volumes (0.25) and pitch (1.0)
    for (int t = 0; t < NUM_TRACKS; t++) {
        _trackVolumes[t] = 0.25f;
        _trackPitch[t] = 1.0f;
    }
    _controlActive = false;
    _controlWasUsed = false;
}

Live::~Live() {
    // Cleanup if needed
}

void Live::refresh() {
    currentState = LIVE_MAIN;
    _selectedTrackIndex = 0;
    _currentPage = 0;
    _stepRange = 0;
    _displayNeedsUpdate = false;

    if (_audioResources) {
        _audioResources->disableLivePassthrough();
    }

    // Load all SD files into memory when entering Live mode
    loadFileList();

    _liveScreen.drawMainView(
        _sequencer.getGrid(), _selectedTrackIndex, _sequencer.getCurrentStep(),
        _sequencer.getBPM(), _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS,
        _currentPage, _stepRange, TRACK_LABELS, _tracks,
        _trackVolumes[_selectedTrackIndex], _trackPitch[_selectedTrackIndex]);

    // Cache all assigned samples when entering Live mode
    cacheSamples();
}

void Live::updateDisplay() {
    if (_displayNeedsUpdate && currentState == LIVE_MAIN) {
        _displayNeedsUpdate = false;
        _liveScreen.drawMainView(_sequencer.getGrid(), _selectedTrackIndex,
                                 _sequencer.getCurrentStep(),
                                 _sequencer.getBPM(), _sequencer.isPlaying(),
                                 NUM_TRACKS, NUM_STEPS, _currentPage,
                                 _stepRange, TRACK_LABELS, _tracks,
                                 _trackVolumes[_selectedTrackIndex],
                                 _trackPitch[_selectedTrackIndex]);
        updateLEDs();
    }
}

void Live::setAudioResources(AudioResources* audioResources) {
    _audioResources = audioResources;

    // Sync mixer gains with stored volumes
    if (_audioResources) {
        for (int i = 0; i < NUM_TRACKS; i++) {
            if (i < 4) {
                _audioResources->mixer4.gain(i, _trackVolumes[i]);
            } else {
                _audioResources->mixer4_2.gain(i - 4, _trackVolumes[i]);
            }
        }
    }
}

long Live::receiveTimerTick() {
    // Keep playing even in sample select mode
    if (_sequencer.isPlaying() &&
        (currentState == LIVE_MAIN || currentState == LIVE_SAMPLE_SELECT)) {
        advanceStep();
        return calculateStepIntervalMicros();
    }
    return 1000000;  // 1s when not playing sequencer
}

void Live::handleEvent(Controls::ButtonEvent event) {
    // State: LIVE_MAIN
    if (currentState == LIVE_MAIN) {
        // Button 5 press/release - control LEDs and detect B1-B4 combos
        if (event.buttonId == 5) {
            if (event.state == PRESSED) {
                // Check for B1-B4 held + B5 pressed combo (set step on ALL
                // pages)
                if (event.button1Held || event.button2Held ||
                    event.button3Held || event.button4Held) {
                    _button5UsedForCombo = true;

                    // Clear the "was pressed" flags to prevent regular toggle
                    _button1WasPressed = false;
                    _button2WasPressed = false;
                    _button3WasPressed = false;
                    _button4WasPressed = false;

                    // Determine which buttons are held and toggle those steps
                    // on ALL pages
                    for (int btn = 1; btn <= 4; btn++) {
                        bool isHeld = false;
                        if (btn == 1)
                            isHeld = event.button1Held;
                        else if (btn == 2)
                            isHeld = event.button2Held;
                        else if (btn == 3)
                            isHeld = event.button3Held;
                        else if (btn == 4)
                            isHeld = event.button4Held;

                        if (isHeld) {
                            // Apply to all 8 pages (32 steps total)
                            int stepOffset = btn - 1;
                            for (int page = 0; page < 8; page++) {
                                int targetStep = (page * 4) + stepOffset;
                                toggleStep(_selectedTrackIndex, targetStep);
                            }
                        }
                    }

                    // Update display
                    _stepRange = (_currentPage >= 4) ? 16 : 0;
                    _liveScreen.drawMainView(
                        _sequencer.getGrid(), _selectedTrackIndex,
                        _sequencer.getCurrentStep(), _sequencer.getBPM(),
                        _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS,
                        _currentPage, _stepRange, TRACK_LABELS, _tracks,
                        _trackVolumes[_selectedTrackIndex],
                        _trackPitch[_selectedTrackIndex]);
                    updateLEDs();
                    return;
                }
            } else {
                // Reset combo flag on release
                _button5UsedForCombo = false;
            }
            return;
        }

        // === VOLUME AND PITCH CONTROL ===
        if (event.buttonId == 0 && event.encoderValue != 0) {
            // Button 1 Held: Change VOLUME of CURRENTLY SELECTED track
            if (event.button1Held) {
                _controlActive = true;
                _controlWasUsed = true;

                float newVolume = _trackVolumes[_selectedTrackIndex] +
                                  (event.encoderValue * VOLUME_STEP);

                // Snap to 0.25 if close (within half a step)
                if (abs(newVolume - 0.25f) < (VOLUME_STEP / 2.0f)) {
                    newVolume = 0.25f;
                }

                setTrackVolume(_selectedTrackIndex, newVolume);

                _displayNeedsUpdate = true;

                // Redraw with volume info (Control Type 1)
                _liveScreen.drawMainView(
                    _sequencer.getGrid(), _selectedTrackIndex,
                    _sequencer.getCurrentStep(), _sequencer.getBPM(),
                    _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                    _stepRange, TRACK_LABELS, _tracks,
                    _trackVolumes[_selectedTrackIndex],
                    _trackPitch[_selectedTrackIndex]);
                return;
            }

            // Button 2 Held: Change PITCH of CURRENTLY SELECTED track
            if (event.button2Held) {
                _controlActive = true;
                _controlWasUsed = true;

                float currentPitch = _trackPitch[_selectedTrackIndex];
                float newPitch =
                    currentPitch + (event.encoderValue * PITCH_STEP);

                // Snap to 1.0 if close (within half a step)
                if (abs(newPitch - 1.0f) < (PITCH_STEP / 2.0f)) {
                    newPitch = 1.0f;
                }

                setTrackPitch(_selectedTrackIndex, newPitch);

                _displayNeedsUpdate = true;

                // Redraw with pitch info (Control Type 2)
                _liveScreen.drawMainView(
                    _sequencer.getGrid(), _selectedTrackIndex,
                    _sequencer.getCurrentStep(), _sequencer.getBPM(),
                    _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                    _stepRange, TRACK_LABELS, _tracks,
                    _trackVolumes[_selectedTrackIndex],
                    _trackPitch[_selectedTrackIndex]);
                return;
            }

            // Button 4 Held: Change BPM
            if (event.button4Held) {
                _controlActive = true;
                _controlWasUsed = true;

                int currentBPM = _sequencer.getBPM();
                int newBPM = currentBPM + event.encoderValue;
                _sequencer.setBPM(newBPM);

                if (_sequencer.isPlaying() && _timerCallback) {
                    _timerCallback(calculateStepIntervalMicros());
                }

                _displayNeedsUpdate = true;

                // Redraw
                _liveScreen.drawMainView(
                    _sequencer.getGrid(), _selectedTrackIndex,
                    _sequencer.getCurrentStep(), _sequencer.getBPM(),
                    _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                    _stepRange, TRACK_LABELS, _tracks,
                    _trackVolumes[_selectedTrackIndex],
                    _trackPitch[_selectedTrackIndex]);
                return;
            }

            if (_controlActive) {
                _controlActive = false;
            }
        }

        // Reset control state on button release (1, 2, or 4)
        if (((event.buttonId >= 1 && event.buttonId <= 2) ||
             event.buttonId == 4) &&
            event.state == NOT_PRESSED) {
            _controlActive = false;
            // _controlWasUsed stays true to prevent toggle
            updateLEDs();
            _liveScreen.drawMainView(
                _sequencer.getGrid(), _selectedTrackIndex,
                _sequencer.getCurrentStep(), _sequencer.getBPM(),
                _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                _stepRange, TRACK_LABELS, _tracks,
                _trackVolumes[_selectedTrackIndex],
                _trackPitch[_selectedTrackIndex]);
        }

        // Encoder rotation - navigate tracks (only when button 5 not held)
        if (event.buttonId == 0 && event.encoderValue != 0 &&
            !event.button5Held) {
            _selectedTrackIndex -= event.encoderValue;
            _selectedTrackIndex =
                constrain(_selectedTrackIndex, 0, NUM_TRACKS - 1);
            _currentPage = 0;  // Reset page when changing track
            _liveScreen.drawMainView(
                _sequencer.getGrid(), _selectedTrackIndex,
                _sequencer.getCurrentStep(), _sequencer.getBPM(),
                _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                _stepRange, TRACK_LABELS, _tracks,
                _trackVolumes[_selectedTrackIndex],
                _trackPitch[_selectedTrackIndex]);
            updateLEDs();
            return;
        }

        // Button 5 held combinations
        if (event.button5Held) {
            // Button 5 + Encoder - change page (0-7)
            if (event.buttonId == 0 && event.encoderValue != 0) {
                _currentPage += event.encoderValue;
                _currentPage = constrain(_currentPage, 0, 7);

                // Update step range based on current page
                _stepRange = (_currentPage >= 4) ? 16 : 0;

                _liveScreen.drawMainView(
                    _sequencer.getGrid(), _selectedTrackIndex,
                    _sequencer.getCurrentStep(), _sequencer.getBPM(),
                    _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                    _stepRange, TRACK_LABELS, _tracks,
                    _trackVolumes[_selectedTrackIndex],
                    _trackPitch[_selectedTrackIndex]);
                updateLEDs();
                return;
            }

            // Button 5 + Button 1 - show file selector
            if (event.buttonId == 1 && event.state == PRESSED) {
                currentState = LIVE_SAMPLE_SELECT;
                _selectedFileIndex = 0;
                _liveScreen.drawSampleSelect(_fileList, _selectedFileIndex,
                                             _fileCount);
                return;
            }
            // Button 5 + Button 2 - exit to home
            if (event.buttonId == 2 && event.state == PRESSED) {
                if (_navCallback) {
                    _navCallback(AppContext::HOME);
                }
                return;
            }
            // Button 5 + Button 4 - toggle play/pause
            if (event.buttonId == 4 && event.state == PRESSED) {
                if (_sequencer.isPlaying()) {
                    stopPlayback();
                } else {
                    startPlayback();
                }
                _liveScreen.drawMainView(
                    _sequencer.getGrid(), _selectedTrackIndex,
                    _sequencer.getCurrentStep(), _sequencer.getBPM(),
                    _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                    _stepRange, TRACK_LABELS, _tracks,
                    _trackVolumes[_selectedTrackIndex],
                    _trackPitch[_selectedTrackIndex]);
                return;
            }
        }

        // Buttons 1-4 PRESS - track that they were pressed (when B5 not already
        // held)
        if (event.buttonId >= 1 && event.buttonId <= 4 &&
            event.state == PRESSED) {
            if (!event.button5Held) {
                // Track that this button was pressed without B5 held
                switch (event.buttonId) {
                    case 1:
                        _button1WasPressed = true;
                        break;
                    case 2:
                        _button2WasPressed = true;
                        break;
                    case 3:
                        _button3WasPressed = true;
                        break;
                    case 4:
                        _button4WasPressed = true;
                        break;
                }
            }
            return;
        }

        // Buttons 1-4 RELEASE - toggle steps (only if B5 wasn't used for combo)
        if (event.buttonId >= 1 && event.buttonId <= 4 &&
            event.state == NOT_PRESSED && !event.button5Held) {
            bool shouldToggle = false;
            switch (event.buttonId) {
                case 1:
                    shouldToggle = _button1WasPressed;
                    _button1WasPressed = false;
                    break;
                case 2:
                    shouldToggle = _button2WasPressed;
                    _button2WasPressed = false;
                    break;
                case 3:
                    shouldToggle = _button3WasPressed;
                    _button3WasPressed = false;
                    break;
                case 4:
                    shouldToggle = _button4WasPressed;
                    _button4WasPressed = false;
                    break;
            }

            if (shouldToggle && !_button5UsedForCombo && !_controlWasUsed) {
                // Calculate step index based on current page (not just
                // stepRange)
                int stepIndex = (_currentPage * 4) + (event.buttonId - 1);

                toggleStep(_selectedTrackIndex, stepIndex);

                // Ensure display matches where we are
                _stepRange = (_currentPage >= 4) ? 16 : 0;

                _liveScreen.drawMainView(
                    _sequencer.getGrid(), _selectedTrackIndex,
                    _sequencer.getCurrentStep(), _sequencer.getBPM(),
                    _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                    _stepRange, TRACK_LABELS, _tracks,
                    _trackVolumes[_selectedTrackIndex],
                    _trackPitch[_selectedTrackIndex]);
                updateLEDs();
            }
            // Reset volume control flag after button release
            _controlWasUsed = false;
            return;
        }
    }

    // State: LIVE_SAMPLE_SELECT
    else if (currentState == LIVE_SAMPLE_SELECT) {
        // Encoder rotation - navigate files
        if (event.buttonId == 0 && event.encoderValue != 0) {
            _selectedFileIndex -= event.encoderValue;
            _selectedFileIndex =
                constrain(_selectedFileIndex, 0, max(0, _fileCount - 1));
            _liveScreen.drawSampleSelect(_fileList, _selectedFileIndex,
                                         _fileCount);
            return;
        }

        // Button 1 (back) - return to main view
        if (event.buttonId == 1 && event.state == PRESSED) {
            currentState = LIVE_MAIN;
            _liveScreen.drawMainView(
                _sequencer.getGrid(), _selectedTrackIndex,
                _sequencer.getCurrentStep(), _sequencer.getBPM(),
                _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                _stepRange, TRACK_LABELS, _tracks,
                _trackVolumes[_selectedTrackIndex],
                _trackPitch[_selectedTrackIndex]);
            updateLEDs();
            return;
        }

        // Button 4 (confirm) - assign sample
        if (event.buttonId == 4 && event.state == PRESSED) {
            assignSampleToTrack();
            currentState = LIVE_MAIN;
            _liveScreen.drawMainView(
                _sequencer.getGrid(), _selectedTrackIndex,
                _sequencer.getCurrentStep(), _sequencer.getBPM(),
                _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                _stepRange, TRACK_LABELS, _tracks,
                _trackVolumes[_selectedTrackIndex],
                _trackPitch[_selectedTrackIndex]);
            updateLEDs();
            return;
        }

        // Button 3 (action) - clear track
        if (event.buttonId == 3 && event.state == PRESSED) {
            clearTrack(_selectedTrackIndex);
            currentState = LIVE_MAIN;
            _liveScreen.drawMainView(
                _sequencer.getGrid(), _selectedTrackIndex,
                _sequencer.getCurrentStep(), _sequencer.getBPM(),
                _sequencer.isPlaying(), NUM_TRACKS, NUM_STEPS, _currentPage,
                _stepRange, TRACK_LABELS, _tracks,
                _trackVolumes[_selectedTrackIndex],
                _trackPitch[_selectedTrackIndex]);
            updateLEDs();
            return;
        }
    }
}

void Live::loadFileList() {
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
    while (true && _fileCount < MAX_FILES) {
        File entry = recordingsDir.openNextFile();
        if (!entry) break;

        String filename = entry.name();
        if (!entry.isDirectory()) {
            // Skip hidden files (e.g. ._REC001.wav)
            if (filename.startsWith(".")) {
                entry.close();
                continue;
            }

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

void Live::assignSampleToTrack() {
    if (_selectedFileIndex >= _fileCount) return;

    String fullFileName = _fileList[_selectedFileIndex];
    _tracks[_selectedTrackIndex].assignSample(fullFileName);

    Serial.print("Assigned '");
    Serial.print(fullFileName);
    Serial.print("' to track ");
    Serial.println(_selectedTrackIndex);
}

void Live::clearTrack(int trackIndex) {
    if (trackIndex < 0 || trackIndex >= NUM_TRACKS) return;

    stopTrack(trackIndex);
    _tracks[trackIndex].clear();

    Serial.print("Cleared track ");
    Serial.println(trackIndex);
}

void Live::playTrack(int trackIndex) {
    if (!_audioResources || trackIndex < 0 || trackIndex >= NUM_TRACKS) return;
    if (!_tracks[trackIndex].isAssigned) return;

    // Get the appropriate WAV player for this track
    AudioPlaySdResmp* player = nullptr;
    switch (trackIndex) {
        case 0:
            player = &_audioResources->playSdWav;
            break;
        case 1:
            player = &_audioResources->playSdWav1;
            break;
        case 2:
            player = &_audioResources->playSdWav2;
            break;
        case 3:
            player = &_audioResources->playSdWav3;
            break;
        case 4:
            player = &_audioResources->playSdWav4;
            break;
        case 5:
            player = &_audioResources->playSdWav5;
            break;
        case 6:
            player = &_audioResources->playSdWav6;
            break;
    }

    if (player) {
        // String wavPath = _tracks[trackIndex].getWavPath();
        String wavPath = _tracks[trackIndex].fileName;
        String fullPath = _tracks[trackIndex].getWavPath();

        AudioNoInterrupts();

        // Set playback rate before playing
        player->setPlaybackRate(_trackPitch[trackIndex]);

        // If it's the same file and already playing, maybe we just want to
        // restart or retrigger? Standard drum machine behavior: re-trigger.
        if (player->isPlaying()) {
            player->stop();
        }

        player->playWav(fullPath.c_str());

        // Trigger envelope to allow audio through
        AudioEffectEnvelope* env = _audioResources->getEnvelope(trackIndex);
        if (env) {
            env->noteOn();
        }
        AudioInterrupts();
    }
}

void Live::stopTrack(int trackIndex) {
    if (!_audioResources || trackIndex < 0 || trackIndex >= NUM_TRACKS) return;

    AudioPlaySdResmp* player = nullptr;
    switch (trackIndex) {
        case 0:
            player = &_audioResources->playSdWav;
            break;
        case 1:
            player = &_audioResources->playSdWav1;
            break;
        case 2:
            player = &_audioResources->playSdWav2;
            break;
        case 3:
            player = &_audioResources->playSdWav3;
            break;
        case 4:
            player = &_audioResources->playSdWav4;
            break;
        case 5:
            player = &_audioResources->playSdWav5;
            break;
        case 6:
            player = &_audioResources->playSdWav6;
            break;
    }

    if (player && player->isPlaying()) {
        player->stop();

        // Release envelope gate
        AudioEffectEnvelope* env = _audioResources->getEnvelope(trackIndex);
        if (env) {
            env->noteOff();
        }
    }
}

void Live::stopAllTracks() {
    for (int i = 0; i < NUM_TRACKS; i++) {
        stopTrack(i);
    }
}

void Live::startPlayback() {
    _sequencer.start();
    // Play the first step immediately and reset the timer
    advanceStep();
    if (_timerCallback) {
        _timerCallback(calculateStepIntervalMicros());
    }
}

void Live::stopPlayback() {
    _sequencer.stop();
    stopAllTracks();
}

void Live::toggleStep(int track, int step) {
    _sequencer.toggleStep(track, step);
}

void Live::advanceStep() {
    int _currentStep = _sequencer.advance();

    // Trigger tracks with enabled steps (audio logic only - no display)
    for (int t = 0; t < NUM_TRACKS; t++) {
        if (_sequencer.isTrackActive(t) && _tracks[t].isAssigned) {
            playTrack(t);
        }
    }

    // Mark display for update (decoupled from audio timing)
    _displayNeedsUpdate = true;

    // Log memory usage periodically (non-blocking, only logs every 5 seconds)
    logMemoryUsage();
}

int Live::calculateStepIntervalMicros() {
    return _sequencer.calculateStepIntervalMicros();
}

String Live::getFileNameWithoutExtension(const String& fileName) {
    int dotIndex = fileName.lastIndexOf('.');
    if (dotIndex > 0) {
        return fileName.substring(0, dotIndex);
    }
    return fileName;
}

void Live::updateLEDs() {
    // Calculate which 4 steps are visible based on current page
    int pageStartStep = _currentPage * 4;

    for (int i = 0; i < 4; i++) {
        int stepIndex = pageStartStep + i;
        int buttonId = i + 1;  // Buttons 1-4

        bool stepEnabled = _sequencer.getStep(_selectedTrackIndex, stepIndex);

        if (stepEnabled) {
            // Solid on
            _keyboard->triggerLedForButton(buttonId, true);
        } else {
            // Off
            _keyboard->triggerLedForButton(buttonId, false);
        }
    }
}

void Live::setTrackVolume(int trackIndex, float volume) {
    if (trackIndex < 0 || trackIndex >= NUM_TRACKS) return;

    _trackVolumes[trackIndex] = constrain(volume, VOLUME_MIN, VOLUME_MAX);

    // Apply to mixer if audio resources are available
    if (_audioResources) {
        if (trackIndex < 4) {
            _audioResources->mixer4.gain(trackIndex, _trackVolumes[trackIndex]);
        } else {
            _audioResources->mixer4_2.gain(trackIndex - 4,
                                           _trackVolumes[trackIndex]);
        }
    }
}

float Live::getTrackVolume(int trackIndex) const {
    if (trackIndex < 0 || trackIndex >= NUM_TRACKS) return 0.5f;
    return _trackVolumes[trackIndex];
}

void Live::setTrackPitch(int trackIndex, float pitch) {
    if (trackIndex < 0 || trackIndex >= NUM_TRACKS) return;
    _trackPitch[trackIndex] = constrain(pitch, PITCH_MIN, PITCH_MAX);
}

float Live::getTrackPitch(int trackIndex) const {
    if (trackIndex < 0 || trackIndex >= NUM_TRACKS) return 1.0f;
    return _trackPitch[trackIndex];
}

void Live::logMemoryUsage() {
    unsigned long currentTime = millis();

    // Log every MEMORY_LOG_INTERVAL ms
    if (currentTime - _lastMemoryLogTime >= MEMORY_LOG_INTERVAL) {
        _lastMemoryLogTime = currentTime;

        // Audio memory usage
        int audioMemoryUsage = AudioMemoryUsage();
        int audioMemoryMax = AudioMemoryUsageMax();

        Serial.print("MEMORY: Audio=");
        Serial.print(audioMemoryUsage);
        Serial.print("blks (max=");
        Serial.print(audioMemoryMax);
        Serial.println(" blks)");
    }
}

void Live::cacheSamples() {
    Serial.println("Caching samples for all assigned tracks...");

    for (int t = 0; t < NUM_TRACKS; t++) {
        if (_tracks[t].isAssigned) {
            String path = _tracks[t].getWavPath();
            char pathBuf[64];
            path.toCharArray(pathBuf, sizeof(pathBuf));

            // Verify file exists on SD card
            if (SD.exists(pathBuf)) {
                Serial.print("Cached track ");
                Serial.print(t);
                Serial.print(": ");
                Serial.println(pathBuf);
            } else {
                Serial.print("Failed to cache track ");
                Serial.print(t);
                Serial.print(": ");
                Serial.println(pathBuf);
            }
        }
    }

    logMemoryUsage();
}
