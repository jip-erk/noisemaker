#include "Live.h"

// Track labels for display
const char* Live::TRACK_LABELS[NUM_TRACKS] = {"A", "B", "C", "D"};

Live::Live(Controls* keyboard, Screen* screen, NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
    _audioResources = nullptr;
    _liveScreen = LiveScreen(screen);

    // Initialize sequencer grid - all steps off
    for (int t = 0; t < NUM_TRACKS; t++) {
        for (int s = 0; s < NUM_STEPS; s++) {
            _sequencerGrid[t][s] = false;
        }
    }

    _currentStep = 0;
    _currentBPM = 120;
    _isPlaying = false;

    // Initialize track volumes to match AudioResources default (0.5)
    for (int t = 0; t < NUM_TRACKS; t++) {
        _trackVolumes[t] = 0.5f;
    }
    _activeVolumeTrack = -1;
    _volumeControlActive = false;
    _volumeControlWasUsed = false;
}

Live::~Live() {
    // Cleanup if needed
}

void Live::refresh() {
    currentState = LIVE_MAIN;
    _selectedTrackIndex = 0;
    _currentPage = 0;
    _displayNeedsUpdate = false;

    // Load all SD files into memory when entering Live mode
    loadFileList();

    _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex, _currentStep,
                             _currentBPM, _isPlaying, NUM_TRACKS, NUM_STEPS,
                             _currentPage, TRACK_LABELS, _tracks);

    // Cache all assigned samples when entering Live mode
    cacheSamples();
}

void Live::updateDisplay() {
    if (_displayNeedsUpdate && currentState == LIVE_MAIN) {
        _displayNeedsUpdate = false;
        _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex,
                                 _currentStep, _currentBPM, _isPlaying,
                                 NUM_TRACKS, NUM_STEPS, _currentPage,
                                 TRACK_LABELS, _tracks);
        updateLEDs();
    }
}

void Live::setAudioResources(AudioResources* audioResources) {
    _audioResources = audioResources;

    // Sync mixer gains with stored volumes
    if (_audioResources) {
        for (int i = 0; i < NUM_TRACKS; i++) {
            _audioResources->mixer4.gain(i, _trackVolumes[i]);
        }
    }
}

long Live::receiveTimerTick() {
    // Keep playing even in sample select mode
    if (_isPlaying &&
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
                // Check for B1-B4 held + B5 pressed combo (navigate to page)
                if (event.button1Held || event.button2Held ||
                    event.button3Held || event.button4Held) {
                    int targetPage = -1;
                    if (event.button1Held)
                        targetPage = 0;
                    else if (event.button2Held)
                        targetPage = 1;
                    else if (event.button3Held)
                        targetPage = 2;
                    else if (event.button4Held)
                        targetPage = 3;

                    if (targetPage != -1) {
                        _button5UsedForCombo = true;  // Mark that B5 was used

                        // Clear the "was pressed" flags to prevent step toggle
                        _button1WasPressed = false;
                        _button2WasPressed = false;
                        _button3WasPressed = false;
                        _button4WasPressed = false;

                        // Only navigate and redraw if page actually changes
                        if (targetPage != _currentPage) {
                            _currentPage = targetPage;
                            _liveScreen.drawMainView(
                                _sequencerGrid, _selectedTrackIndex,
                                _currentStep, _currentBPM, _isPlaying,
                                NUM_TRACKS, NUM_STEPS, _currentPage,
                                TRACK_LABELS, _tracks);
                            updateLEDs();
                        }
                        return;
                    }
                }

                // No combo - turn off all LEDs when button 5 is pressed alone
                for (int i = 1; i <= 4; i++) {
                    _keyboard->triggerLedForButton(i, false);
                }
            } else {
                // Reset combo flag on release
                _button5UsedForCombo = false;
                // Restore LED state when button 5 is released
                updateLEDs();
            }
            return;
        }

        // === VOLUME CONTROL: B1-B4 held + Encoder rotation ===
        if (event.buttonId == 0 && event.encoderValue != 0) {
            int volumeTrack = -1;

            // Determine which track volume to adjust (priority: B1 > B2 > B3 >
            // B4)
            if (event.button1Held)
                volumeTrack = 0;
            else if (event.button2Held)
                volumeTrack = 1;
            else if (event.button3Held)
                volumeTrack = 2;
            else if (event.button4Held)
                volumeTrack = 3;

            if (volumeTrack >= 0) {
                // Volume control is active
                _volumeControlActive = true;
                _volumeControlWasUsed = true;  // Prevent step toggle on release
                _activeVolumeTrack = volumeTrack;

                // Adjust volume
                float newVolume = _trackVolumes[volumeTrack] +
                                  (event.encoderValue * VOLUME_STEP);
                newVolume = constrain(newVolume, VOLUME_MIN, VOLUME_MAX);

                setTrackVolume(volumeTrack, newVolume);

                // Update display to show volume change
                _displayNeedsUpdate = true;

                Serial.print("Track ");
                Serial.print(volumeTrack);
                Serial.print(" volume: ");
                Serial.println(newVolume);

                // Redraw screen with volume display
                _liveScreen.drawMainView(
                    _sequencerGrid, _selectedTrackIndex, _currentStep,
                    _currentBPM, _isPlaying, NUM_TRACKS, NUM_STEPS,
                    _currentPage, TRACK_LABELS, _tracks, _activeVolumeTrack,
                    _trackVolumes[_activeVolumeTrack]);

                return;  // Exit early - don't process as normal encoder
                         // navigation
            } else {
                // No button held - reset volume control state
                _volumeControlActive = false;
                _activeVolumeTrack = -1;
            }
        }

        // Reset volume control state on button release
        if (event.buttonId >= 1 && event.buttonId <= 4 &&
            event.state == NOT_PRESSED) {
            if (_activeVolumeTrack == (event.buttonId - 1)) {
                _volumeControlActive = false;
                _activeVolumeTrack = -1;
                // _volumeControlWasUsed stays true to prevent step toggle
                // Restore all LEDs to sequencer pattern
                updateLEDs();
                // Redraw without volume display
                _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex,
                                         _currentStep, _currentBPM, _isPlaying,
                                         NUM_TRACKS, NUM_STEPS, _currentPage,
                                         TRACK_LABELS, _tracks);
            }
        }

        // Encoder rotation - navigate tracks (only when button 5 not held)
        if (event.buttonId == 0 && event.encoderValue != 0 &&
            !event.button5Held) {
            _selectedTrackIndex -= event.encoderValue;
            _selectedTrackIndex =
                constrain(_selectedTrackIndex, 0, NUM_TRACKS - 1);
            _currentPage = 0;  // Reset page when changing track
            _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex,
                                     _currentStep, _currentBPM, _isPlaying,
                                     NUM_TRACKS, NUM_STEPS, _currentPage,
                                     TRACK_LABELS, _tracks);
            updateLEDs();
            return;
        }

        // Button 5 held combinations
        if (event.button5Held) {
            // Button 5 + Encoder - change pages
            if (event.buttonId == 0 && event.encoderValue != 0) {
                _currentPage += event.encoderValue;
                _currentPage = constrain(_currentPage, 0, 3);
                _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex,
                                         _currentStep, _currentBPM, _isPlaying,
                                         NUM_TRACKS, NUM_STEPS, _currentPage,
                                         TRACK_LABELS, _tracks);
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
                if (_isPlaying) {
                    stopPlayback();
                } else {
                    startPlayback();
                }
                _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex,
                                         _currentStep, _currentBPM, _isPlaying,
                                         NUM_TRACKS, NUM_STEPS, _currentPage,
                                         TRACK_LABELS, _tracks);
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

            if (shouldToggle && !_button5UsedForCombo &&
                !_volumeControlWasUsed) {
                int stepIndex = (_currentPage * 4) + (event.buttonId - 1);
                toggleStep(_selectedTrackIndex, stepIndex);
                _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex,
                                         _currentStep, _currentBPM, _isPlaying,
                                         NUM_TRACKS, NUM_STEPS, _currentPage,
                                         TRACK_LABELS, _tracks);
                updateLEDs();
            }
            // Reset volume control flag after button release
            _volumeControlWasUsed = false;
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
            _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex,
                                     _currentStep, _currentBPM, _isPlaying,
                                     NUM_TRACKS, NUM_STEPS, _currentPage,
                                     TRACK_LABELS, _tracks);
            updateLEDs();
            return;
        }

        // Button 4 (confirm) - assign sample
        if (event.buttonId == 4 && event.state == PRESSED) {
            assignSampleToTrack();
            currentState = LIVE_MAIN;
            _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex,
                                     _currentStep, _currentBPM, _isPlaying,
                                     NUM_TRACKS, NUM_STEPS, _currentPage,
                                     TRACK_LABELS, _tracks);
            updateLEDs();
            return;
        }

        // Button 3 (action) - clear track
        if (event.buttonId == 3 && event.state == PRESSED) {
            clearTrack(_selectedTrackIndex);
            currentState = LIVE_MAIN;
            _liveScreen.drawMainView(_sequencerGrid, _selectedTrackIndex,
                                     _currentStep, _currentBPM, _isPlaying,
                                     NUM_TRACKS, NUM_STEPS, _currentPage,
                                     TRACK_LABELS, _tracks);
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
    AudioPlaySdWav* player = nullptr;
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
    }

    if (player) {
        String wavPath = _tracks[trackIndex].getWavPath();

        AudioNoInterrupts();

        player->play(wavPath.c_str());
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

    AudioPlaySdWav* player = nullptr;
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
    _isPlaying = true;
    _currentStep = -1;  // Will advance to 0 on first tick
}

void Live::stopPlayback() {
    _isPlaying = false;
    stopAllTracks();
}

void Live::toggleStep(int track, int step) {
    if (track >= 0 && track < NUM_TRACKS && step >= 0 && step < NUM_STEPS) {
        _sequencerGrid[track][step] = !_sequencerGrid[track][step];
    }
}

void Live::advanceStep() {
    _currentStep = (_currentStep + 1) % NUM_STEPS;

    // Trigger tracks with enabled steps (audio logic only - no display)
    for (int t = 0; t < NUM_TRACKS; t++) {
        if (_sequencerGrid[t][_currentStep] && _tracks[t].isAssigned) {
            playTrack(t);
        }
    }

    // Mark display for update (decoupled from audio timing)
    _displayNeedsUpdate = true;

    // Log memory usage periodically (non-blocking, only logs every 5 seconds)
    logMemoryUsage();
}

int Live::calculateStepIntervalMicros() {
    // 16th note = (60 / BPM / 4) seconds = (60000000 / BPM / 4) microseconds
    return (60000000 / _currentBPM / 4);
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

        bool stepEnabled = _sequencerGrid[_selectedTrackIndex][stepIndex];

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
        _audioResources->mixer4.gain(trackIndex, _trackVolumes[trackIndex]);
    }
}

float Live::getTrackVolume(int trackIndex) const {
    if (trackIndex < 0 || trackIndex >= NUM_TRACKS) return 0.5f;
    return _trackVolumes[trackIndex];
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
