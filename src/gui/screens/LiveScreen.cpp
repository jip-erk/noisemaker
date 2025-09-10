#include "LiveScreen.h"

LiveScreen::LiveScreen(Controls *keyboard, Screen *screen,
                       NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
    _audioResources = nullptr; // Will be set externally
}

LiveScreen::~LiveScreen() {
    // Cleanup if needed
}

void LiveScreen::refresh() {
    currentState = LIVE_HOME;
    loadFileList();
    
    _screen->clear();
    _screen->drawStr(0, 8, "Live Samples");
    
    if (_fileCount == 0) {
        _screen->drawStr(0, 30, "No samples found");
        _screen->drawStr(0, 45, "Record some first!");
    } else {
        drawFileList();
    }
    
    _screen->display();
}

void LiveScreen::setAudioResources(AudioResources* audioResources) {
    _audioResources = audioResources;
}

void LiveScreen::handleEvent(Controls::ButtonEvent event) {
    if (event.buttonId == 1 && event.state == PRESSED) {
        // Back button - return to home
        if (_navCallback) {
            _navCallback(AppContext::HOME);
            return;
        }
    }

    if (event.buttonId == 2 && event.state == PRESSED) {
        // Select/Play button
        if (currentState == LIVE_HOME && _fileCount > 0) {
            playSelectedFile();
        } else if (currentState == LIVE_PLAYING) {
            // Pause playback
            currentState = LIVE_PAUSED;
        } else if (currentState == LIVE_PAUSED) {
            // Resume playback
            currentState = LIVE_PLAYING;
            _playbackStartTime = millis();
        }
        return;
    }

    // Handle encoder for file selection (only on home screen)
    if (event.buttonId == 0 && currentState == LIVE_HOME) {
        if (event.encoderValue != 0) {
            _selectedIndex += event.encoderValue;
            _selectedIndex = constrain(_selectedIndex, 0, _fileCount - 1);
            
            // Redraw file list with new selection
            _screen->clear();
            _screen->drawStr(0, 8, "Live Samples");
            drawFileList();
            _screen->display();
        }
        return;
    }

    // Handle encoder for volume control during playback
    if (event.buttonId == 0 && (currentState == LIVE_PLAYING || currentState == LIVE_PAUSED)) {
        if (_audioResources && event.encoderValue != 0) {
            float volumeChange = event.encoderValue * 0.05; // 5% steps
            _audioResources->currentVolume += volumeChange;
            _audioResources->currentVolume = constrain(_audioResources->currentVolume, 0.0, 1.0);
            _audioResources->audioShield.volume(_audioResources->currentVolume);
        }
        return;
    }

    // Handle stop playback (button 3 or long press on button 2)
    if (event.buttonId == 3 && event.state == PRESSED) {
        if (currentState == LIVE_PLAYING || currentState == LIVE_PAUSED) {
            stopPlayback();
        }
        return;
    }

    if (currentState == LIVE_HOME) refresh();
}

void LiveScreen::loadFileList() {
    _fileCount = 0;
    
    // Check if RECORDINGS directory exists
    if (!SD.exists("/RECORDINGS")) {
        return;
    }
    
    File recordingsDir = SD.open("/RECORDINGS");
    if (!recordingsDir) {
        return;
    }
    
    // Read all .WAV files
    while (recordingsDir.available() && _fileCount < 20) {
        File entry = recordingsDir.openNextFile();
        if (!entry) break;
        
        if (!entry.isDirectory()) {
            String filename = entry.name();
            if (filename.endsWith(".WAV") || filename.endsWith(".wav")) {
                _fileList[_fileCount] = filename;
                _fileCount++;
            }
        }
        entry.close();
    }
    
    recordingsDir.close();
    
    // Reset selection if out of bounds
    if (_selectedIndex >= _fileCount) {
        _selectedIndex = 0;
    }
}

void LiveScreen::playSelectedFile() {
    if (_selectedIndex >= _fileCount || !_audioResources) {
        return;
    }
    
    _currentPlayingFile = _fileList[_selectedIndex];
    currentState = LIVE_PLAYING;
    _playbackStartTime = millis();
    
    // Start playing the WAV file
    String fullPath = "/RECORDINGS/" + _currentPlayingFile;
    if (_audioResources->playWav1.play(fullPath.c_str())) {
        // Switch mixer to playback mode
        _audioResources->mixer1.gain(0, 1.0); // Playback channel
        _audioResources->mixer1.gain(1, 0.0); // Input channel (muted)
        
        _screen->clear();
        _screen->drawStr(0, 8, "Playing:");
        _screen->drawStr(0, 20, _currentPlayingFile.c_str());
        _screen->drawStr(0, 35, "Click to pause");
        _screen->drawStr(0, 50, "Hold to stop");
        _screen->display();
    } else {
        // Failed to play file
        currentState = LIVE_HOME;
        _screen->clear();
        _screen->drawStr(0, 8, "Error playing file");
        _screen->drawStr(0, 25, _currentPlayingFile.c_str());
        _screen->display();
        delay(2000);
        refresh();
    }
}

void LiveScreen::stopPlayback() {
    // Stop audio playback
    if (_audioResources) {
        _audioResources->playWav1.stop();
        // Switch mixer back to input mode
        _audioResources->mixer1.gain(0, 0.0); // Playback channel (muted)
        _audioResources->mixer1.gain(1, 1.0); // Input channel
    }
    
    currentState = LIVE_HOME;
    _currentPlayingFile = "";
    
    _screen->clear();
    _screen->drawStr(0, 8, "Live Samples");
    drawFileList();
    _screen->display();
}

void LiveScreen::updatePlayback() {
    if (currentState != LIVE_PLAYING && currentState != LIVE_PAUSED) {
        return;
    }
    
    // Check if playback finished
    if (currentState == LIVE_PLAYING && _audioResources && !_audioResources->playWav1.isPlaying()) {
        stopPlayback();
        return;
    }
    
    // Update playback display
    _screen->clear();
    _screen->drawStr(0, 8, currentState == LIVE_PLAYING ? "Playing:" : "Paused:");
    _screen->drawStr(0, 20, _currentPlayingFile.c_str());
    
    // Show playback time
    unsigned long elapsed = millis() - _playbackStartTime;
    int seconds = (elapsed / 1000) % 60;
    int minutes = (elapsed / 1000) / 60;
    
    String timeStr = String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
    _screen->drawStr(0, 35, timeStr.c_str());
    
    // Show volume
    if (_audioResources) {
        String volStr = "Vol: " + String((int)(_audioResources->currentVolume * 100)) + "%";
        _screen->drawStr(0, 50, volStr.c_str());
    }
    
    _screen->display();
}

void LiveScreen::drawFileList() {
    if (_fileCount == 0) return;
    
    // Calculate which files to show (scrollable list)
    int startIndex = max(0, _selectedIndex - 2);
    int endIndex = min(_fileCount, startIndex + 5);
    
    int yPos = 20;
    for (int i = startIndex; i < endIndex; i++) {
        if (i == _selectedIndex) {
            // Highlight selected item
            _screen->drawBox(0, yPos - 2, 128, 12);
            _screen->getDisplay()->setDrawColor(0); // Invert text color
        }
        
        // Truncate filename if too long
        String displayName = _fileList[i];
        if (displayName.length() > 15) {
            displayName = displayName.substring(0, 12) + "...";
        }
        
        _screen->drawStr(2, yPos + 8, displayName.c_str());
        
        if (i == _selectedIndex) {
            _screen->getDisplay()->setDrawColor(1); // Reset text color
        }
        
        yPos += 12;
    }
    
    // Show selection indicator
    if (_fileCount > 5) {
        _screen->drawStr(120, 60, "^");
    }
}
