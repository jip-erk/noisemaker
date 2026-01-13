#include "Live.h"

// Track labels for display
const char* Live::TRACK_LABELS[NUM_TRACKS] = {"kick", "snare", "hat", "perc"};

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
}

Live::~Live() {
    // Cleanup if needed
}

void Live::refresh() {
    currentState = LIVE_TRACK_VIEW;
    _selectedTrackIndex = 0;
    _liveScreen.drawTrackView(_tracks, _selectedTrackIndex, NUM_TRACKS,
                              TRACK_LABELS);
}

void Live::setAudioResources(AudioResources* audioResources) {
    _audioResources = audioResources;
}

long Live::receiveTimerTick() {
    if (_isPlaying && currentState == LIVE_SEQUENCER) {
        advanceStep();
        return calculateStepIntervalMicros();
    }
    return 1000000;  // 1s when not playing sequencer
}

void Live::handleEvent(Controls::ButtonEvent event) {
    // Back button (button 1)
    if (event.buttonId == 1 && event.state == PRESSED) {
        if (currentState == LIVE_TRACK_VIEW) {
            // Return to home
            if (_navCallback) {
                _navCallback(AppContext::HOME);
                return;
            }
        } else if (currentState == LIVE_SAMPLE_SELECT) {
            // Return to track view
            currentState = LIVE_TRACK_VIEW;
            _liveScreen.drawTrackView(_tracks, _selectedTrackIndex, NUM_TRACKS,
                                      TRACK_LABELS);
        } else if (currentState == LIVE_SEQUENCER) {
            // Return to track view
            if (_isPlaying) {
                stopPlayback();
            }
            currentState = LIVE_TRACK_VIEW;
            _liveScreen.drawTrackView(_tracks, _selectedTrackIndex, NUM_TRACKS,
                                      TRACK_LABELS);
        }
        return;
    }

    // Button 2 - Select/Confirm
    if (event.buttonId == 2 && event.state == PRESSED) {
        if (currentState == LIVE_TRACK_VIEW) {
            // Enter sample selection for this track
            loadFileList();
            currentState = LIVE_SAMPLE_SELECT;
            _selectedFileIndex = 0;
            _liveScreen.drawSampleSelect(_fileList, _selectedFileIndex,
                                         _fileCount);
        } else if (currentState == LIVE_SAMPLE_SELECT) {
            // Assign selected sample to track
            assignSampleToTrack();
            currentState = LIVE_TRACK_VIEW;
            _liveScreen.drawTrackView(_tracks, _selectedTrackIndex, NUM_TRACKS,
                                      TRACK_LABELS);
        } else if (currentState == LIVE_SEQUENCER) {
            // Toggle step at current playhead position
            toggleStep(_selectedTrackIndex, _currentStep);
            _liveScreen.drawSequencer(_sequencerGrid, _selectedTrackIndex,
                                      _currentStep, _currentBPM, _isPlaying,
                                      NUM_TRACKS, NUM_STEPS);
        }
        return;
    }

    // Button 3 - Action
    if (event.buttonId == 3 && event.state == PRESSED) {
        if (currentState == LIVE_TRACK_VIEW) {
            // Enter sequencer view
            currentState = LIVE_SEQUENCER;
            _liveScreen.drawSequencer(_sequencerGrid, _selectedTrackIndex,
                                      _currentStep, _currentBPM, _isPlaying,
                                      NUM_TRACKS, NUM_STEPS);
        } else if (currentState == LIVE_SAMPLE_SELECT) {
            // Clear the track
            clearTrack(_selectedTrackIndex);
            currentState = LIVE_TRACK_VIEW;
            _liveScreen.drawTrackView(_tracks, _selectedTrackIndex, NUM_TRACKS,
                                      TRACK_LABELS);
        } else if (currentState == LIVE_SEQUENCER) {
            // Toggle playback
            if (_isPlaying) {
                stopPlayback();
            } else {
                startPlayback();
            }
            _liveScreen.drawSequencer(_sequencerGrid, _selectedTrackIndex,
                                      _currentStep, _currentBPM, _isPlaying,
                                      NUM_TRACKS, NUM_STEPS);
        }
        return;
    }

    // Encoder - Navigation
    if (event.buttonId == 0 && event.encoderValue != 0) {
        if (currentState == LIVE_TRACK_VIEW) {
            // Navigate tracks
            _selectedTrackIndex += event.encoderValue;
            _selectedTrackIndex = constrain(_selectedTrackIndex, 0, NUM_TRACKS - 1);
            _liveScreen.drawTrackView(_tracks, _selectedTrackIndex, NUM_TRACKS,
                                      TRACK_LABELS);
        } else if (currentState == LIVE_SAMPLE_SELECT) {
            // Navigate files
            _selectedFileIndex += event.encoderValue;
            _selectedFileIndex =
                constrain(_selectedFileIndex, 0, max(0, _fileCount - 1));
            _liveScreen.drawSampleSelect(_fileList, _selectedFileIndex,
                                         _fileCount);
        } else if (currentState == LIVE_SEQUENCER) {
            // Navigate tracks (row-first)
            _selectedTrackIndex += event.encoderValue;
            _selectedTrackIndex = constrain(_selectedTrackIndex, 0, NUM_TRACKS - 1);
            _liveScreen.drawSequencer(_sequencerGrid, _selectedTrackIndex,
                                      _currentStep, _currentBPM, _isPlaying,
                                      NUM_TRACKS, NUM_STEPS);
        }
        return;
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

        Serial.print(trackIndex);
        Serial.print(": ");
        Serial.print(wavPath);

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

    // Trigger tracks with enabled steps
    for (int t = 0; t < NUM_TRACKS; t++) {
        if (_sequencerGrid[t][_currentStep] && _tracks[t].isAssigned) {
            playTrack(t);
        }
    }

    // Update display
    if (currentState == LIVE_SEQUENCER) {
        _liveScreen.drawSequencer(_sequencerGrid, _selectedTrackIndex,
                                  _currentStep, _currentBPM, _isPlaying,
                                  NUM_TRACKS, NUM_STEPS);
    }
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
