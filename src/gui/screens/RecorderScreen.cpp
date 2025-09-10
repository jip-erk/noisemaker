#include "RecorderScreen.h"

RecorderScreen::RecorderScreen(Controls *keyboard, Screen *screen,
                               NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
    _audioResources = nullptr; // Will be set externally
    _wavWriter = nullptr; // Will be created when audio resources are set
}

RecorderScreen::~RecorderScreen() {
    if (_wavWriter) {
        delete _wavWriter;
        _wavWriter = nullptr;
    }
}

void RecorderScreen::refresh() {
    currentState = RECORDER_HOME;
    _screen->clear();
    _screen->drawStr(0, 8, "Recorder");
    _screen->drawStr(0, 20, "Click to start");
    drawVolumeBar();
    _screen->display();
}

void RecorderScreen::setAudioResources(AudioResources* audioResources) {
    _audioResources = audioResources;
    // Create WavFileWriter with the audio queue
    _wavWriter = new WavFileWriter(_audioResources->queue1);
}

void RecorderScreen::handleEvent(Controls::ButtonEvent event) {
    if (event.buttonId == 1 && event.state == PRESSED) {
        if (_navCallback) {
            _navCallback(AppContext::HOME);
            return;
        }
    }

    if (event.buttonId == 2 && event.state == PRESSED) {
        if (currentState == RECORDER_HOME) {
            startWaitingForSound();
        } else if (currentState == RECORDER_WAITING_FOR_SOUND) {
            // Cancel waiting and return to home
            stopBuffering(); // Stop buffering when canceling
            currentState = RECORDER_HOME;
            refresh();
        } else if (currentState == RECORDER_RECORDING) {
            // Stop recording on click while recording
            stopRecording();
            refresh();
        }
        return;
    }

    // Handle encoder for threshold adjustment (only on home screen)
    if (event.buttonId == 0 && currentState == RECORDER_HOME) {
        handleEncoder(event.encoderValue);
        return;
    }
}

void RecorderScreen::showRecorderScreen() {
    _screen->clear();
    _screen->drawStr(0, 8, "Recorder");
    _screen->drawBox(0, 12, 128, 38);
    _screen->getDisplay()->setFont(u8g2_font_micro_tr);
    _screen->drawStr(0, 60, "Recording");
    _screen->drawStr(52, 60, "00:00");
    _screen->getDisplay()->setFont(u8g2_font_ncenB08_tr);
    _screen->display();

    startRecording();
}

void RecorderScreen::startRecording() {
    // Check if audio resources and WAV writer are available
    if (!_audioResources || !_wavWriter) {
        return;
    }

    // Create RECORDINGS folder if it doesn't exist
    if (!SD.exists("/RECORDINGS")) {
        SD.mkdir("/RECORDINGS");
    }

    // Generate unique filename with timestamp
    String filename = "/RECORDINGS/RECORD_" + String(millis()) + ".WAV";

    // Start WAV recording
    if (_wavWriter->open(filename.c_str(), 44100, 1)) {
        // Write the buffered audio first (the audio before threshold was reached)
        writeBufferedAudioToFile();
        
        currentState = RECORDER_RECORDING;
        _recordingStartTime = millis();
    }
}

void RecorderScreen::continueRecording() {
    // Check if WAV writer is available and writing
    if (!_wavWriter || !_wavWriter->isWriting()) {
        return;
    }

    // Update the WAV file with new audio data
    _wavWriter->update();
}

void RecorderScreen::stopRecording() {
    if (!_wavWriter || !_wavWriter->isWriting()) {
        return;
    }

    // Close the WAV file
    if (_wavWriter->close()) {
        currentState = RECORDER_HOME;
    }
}

void RecorderScreen::startWaitingForSound() {
    currentState = RECORDER_WAITING_FOR_SOUND;
    _waitingStartTime = millis();
    
    // Start buffering audio immediately
    startBuffering();
    
    _screen->clear();
    _screen->drawStr(0, 8, "Recorder");
    _screen->drawStr(0, 35, "Waiting for sound...");
    _screen->drawStr(0, 50, "Click to cancel");
    _screen->display();
}

void RecorderScreen::checkVolumeThreshold() {
    if (currentState != RECORDER_WAITING_FOR_SOUND || !_audioResources) {
        return;
    }

    // Process audio buffer continuously
    processAudioBuffer();

    // Check if peak analyzer has new data
    if (_audioResources->peak1.available()) {
        float peakLevel = _audioResources->peak1.read();
        
        // Update display with current volume level
        _screen->clear();
        _screen->drawStr(0, 8, "Recorder");
        _screen->drawStr(0, 35, "Waiting for sound...");
        _screen->drawStr(0, 50, "Click to cancel");
        
        // Show current volume level
        String volumeText = "Vol: " + String(peakLevel * 100, 1) + "%";
        _screen->drawStr(0, 25, volumeText.c_str());
        
        // Show threshold line
        String thresholdText = "Need: " + String(_volumeThreshold * 100, 1) + "%";
        _screen->drawStr(60, 25, thresholdText.c_str());
        
        _screen->display();
        
        // If volume exceeds threshold, start recording
        if (peakLevel > _volumeThreshold) {
            currentState = RECORDER_RECORDING;
            stopBuffering(); // Stop buffering and start writing to file
            showRecorderScreen();
        }
    }
}

void RecorderScreen::setVolumeThreshold(float threshold) {
    if (threshold >= 0.0 && threshold <= 1.0) {
        _volumeThreshold = threshold;
    }
}

void RecorderScreen::drawVolumeBar() {
    if (!_audioResources || currentState != RECORDER_HOME) return;
    
    // Small volume bar - 32x8 pixels with border
    int barX = 48;  // Centered on 128px screen
    int barY = 30;
    int barWidth = 32;
    int barHeight = 8;
    
    // Clear the volume bar area first
    _screen->getDisplay()->setDrawColor(0);
    _screen->getDisplay()->drawBox(barX, barY, barWidth, barHeight);
    
    // Draw border
    _screen->getDisplay()->setDrawColor(1);
    _screen->getDisplay()->drawFrame(barX, barY, barWidth, barHeight);
    
    // Calculate current volume bar width
    int currentVolumeWidth = (int)(_currentVolume * (barWidth - 2));
    if (currentVolumeWidth > (barWidth - 2)) currentVolumeWidth = barWidth - 2;
    
    // Draw current volume fill
    if (currentVolumeWidth > 0) {
        _screen->getDisplay()->setDrawColor(1);
        _screen->getDisplay()->drawBox(barX + 1, barY + 1, currentVolumeWidth, barHeight - 2);
    }
    
    // Draw threshold line
    int thresholdX = (int)(_volumeThreshold * (barWidth - 2));
    if (thresholdX >= 0 && thresholdX < (barWidth - 2)) {
        _screen->getDisplay()->drawVLine(barX + 1 + thresholdX, barY + 1, barHeight - 2);
    }
}

void RecorderScreen::updateVolumeBar() {
    if (!_audioResources || currentState != RECORDER_HOME) return;
    
    // Update current volume from audio analyzer
    if (_audioResources->peak1.available()) {
        float newVolume = _audioResources->peak1.read();
        
        // Only update if volume changed significantly to avoid flicker
        static float lastVolume = -1.0;
        if (abs(newVolume - lastVolume) > 0.02) { // Increased threshold to reduce updates
            _currentVolume = newVolume;
            lastVolume = newVolume;
            
            // Only redraw the volume bar area, not the entire screen
            drawVolumeBar();
            _screen->display();
        }
    }
}

void RecorderScreen::handleEncoder(int direction) {
    if (direction == 0) return;
    
    // Adjust threshold based on encoder direction
    float step = 0.01; // 1% steps
    if (direction > 0) {
        _volumeThreshold += step;
    } else {
        _volumeThreshold -= step;
    }
    
    // Clamp threshold between 0 and 1
    if (_volumeThreshold < 0.0) _volumeThreshold = 0.0;
    if (_volumeThreshold > 1.0) _volumeThreshold = 1.0;
    
    // Only redraw the volume bar to show updated threshold
    drawVolumeBar();
    _screen->display();
}

void RecorderScreen::startBuffering() {
    if (!_audioResources) return;
    
    _isBuffering = true;
    _bufferWriteIndex = 0;
    _bufferFull = false;
    
    // Clear the buffer
    memset(_audioBuffer, 0, sizeof(_audioBuffer));
    
    // Start the audio queue
    _audioResources->queue1.begin();
}

void RecorderScreen::stopBuffering() {
    _isBuffering = false;
    _audioResources->queue1.end();
}

void RecorderScreen::processAudioBuffer() {
    if (!_isBuffering || !_audioResources) return;
    
    // Process available audio data
    if (_audioResources->queue1.available() >= 2) {
        // Get audio data from queue
        memcpy(_audioBuffer + _bufferWriteIndex, _audioResources->queue1.readBuffer(), 256);
        _audioResources->queue1.freeBuffer();
        
        _bufferWriteIndex += 128; // 256 bytes = 128 samples (16-bit)
        
        // Wrap around if buffer is full
        if (_bufferWriteIndex >= BUFFER_SIZE) {
            _bufferWriteIndex = 0;
            _bufferFull = true;
        }
    }
}

void RecorderScreen::writeBufferedAudioToFile() {
    if (!_wavWriter || !_wavWriter->isWriting()) return;
    
    // Write the buffered audio to the file
    int samplesToWrite = _bufferFull ? BUFFER_SIZE : _bufferWriteIndex;
    
    for (int i = 0; i < samplesToWrite; i += 128) {
        int samples = min(128, samplesToWrite - i);
        _wavWriter->getFile().write((uint8_t*)(_audioBuffer + i), samples * 2);
    }
}