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
    
    // Debug: Show file count
    String debugText = "Files: " + String(_fileCount);
    _screen->drawStr(0, 20, debugText.c_str());
    
    if (_fileCount == 0) {
        _screen->drawStr(0, 35, "No samples found");
        _screen->drawStr(0, 50, "Record some first!");
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
            
            // Debug: Show file count
            String debugText = "Files: " + String(_fileCount);
            _screen->drawStr(0, 20, debugText.c_str());
            
            if (_fileCount > 0) {
                drawFileList();
            } else {
                _screen->drawStr(0, 35, "No samples found");
                _screen->drawStr(0, 50, "Record some first!");
            }
            _screen->display();
        }
        return;
    }

    // Note: Volume control removed for USB audio - controlled by host system

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
        Serial.println("RECORDINGS directory does not exist");
        return;
    }
    
    File recordingsDir = SD.open("/RECORDINGS");
    if (!recordingsDir) {
        Serial.println("Failed to open RECORDINGS directory");
        return;
    }
    
    if (!recordingsDir.isDirectory()) {
        Serial.println("RECORDINGS is not a directory");
        recordingsDir.close();
        return;
    }
    
    Serial.println("Scanning RECORDINGS directory...");
    
    // Read all .WAV files
    while (true && _fileCount < 20) {
        File entry = recordingsDir.openNextFile();
        if (!entry) {
            Serial.println("No more entries");
            break;
        }
        
        String filename = entry.name();
        Serial.println("Found entry: " + filename + " (isDir: " + String(entry.isDirectory()) + ")");
        
        if (!entry.isDirectory()) {
            if (filename.endsWith(".WAV") || filename.endsWith(".wav")) {
                _fileList[_fileCount] = filename;
                _fileCount++;
                Serial.println("Added to list: " + filename);
            } else {
                Serial.println("Skipped (not WAV): " + filename);
            }
        } else {
            Serial.println("Skipped directory: " + filename);
        }
        entry.close();
    }
    
    recordingsDir.close();
    
    Serial.println("Total files found: " + String(_fileCount));
    
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
        _screen->clear();
        _screen->drawStr(0, 8, "Playing:");
        _screen->drawStr(0, 20, _currentPlayingFile.c_str());
        _screen->drawStr(0, 35, "Click to pause");
        _screen->drawStr(0, 50, "USB Audio");
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
    
    // Show USB audio info
    _screen->drawStr(0, 50, "USB Audio");
    
    _screen->display();
}

void LiveScreen::drawFileList() {
    if (_fileCount == 0) return;
    
    // Calculate which files to show (scrollable list)
    int startIndex = max(0, _selectedIndex - 2);
    int endIndex = min(_fileCount, startIndex + 3); // Reduced to fit with debug text
    
    int yPos = 30; // Start below debug text
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
    if (_fileCount > 3) {
        _screen->drawStr(120, 60, "^");
    }
}
