#include "RecorderScreen.h"

// Timer interval constants (in microseconds)
static const long VOLUME_UPDATE_INTERVAL_US = 70000;     // ~14 Hz
static const long WAVEFORM_UPDATE_INTERVAL_US = 500000;  // 2 Hz
static const long DEFAULT_TICK_INTERVAL_US = 1000000;    // 1 Hz

RecorderScreen::RecorderScreen(Controls* keyboard, Screen* screen,
                               NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
    _audioResources = nullptr;
    _wavWriter = nullptr;
    _volumeBar = VolumeBar(screen, 68, 0, 60, 10);
    _waveform = Waveform(screen, 0, 15, 128, 47);
    _waveformSelector = WaveformSelector(&_waveform);
}

RecorderScreen::~RecorderScreen() {
    if (_wavWriter) {
        delete _wavWriter;
        _wavWriter = nullptr;
    }
}

long RecorderScreen::receiveTimerTick() {
    if (currentState == RECORDER_HOME) {
        updateVolumeBar();
        return VOLUME_UPDATE_INTERVAL_US;
    } else if (currentState == RECORDER_RECORDING) {
        updateWaveform();
        return WAVEFORM_UPDATE_INTERVAL_US;
    }

    return DEFAULT_TICK_INTERVAL_US;
}

void RecorderScreen::refresh() {
    currentState = RECORDER_HOME;
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "RECORDER");
    _screen->setNormalFont();
    _screen->drawStr(0, 20, "Click to start");
    _volumeBar.drawVolumeBar();
    _screen->display();
}

void RecorderScreen::setAudioResources(AudioResources* audioResources) {
    _audioResources = audioResources;
    // Create WavFileWriter with the audio queue
    _wavWriter = new WavFileWriter(_audioResources->queue1);
}

void RecorderScreen::handleEvent(Controls::ButtonEvent event) {
    if (event.buttonId == 1 && event.state == PRESSED) {
        // If in editing mode, save the .bdf file with start/end positions
        if (currentState == RECORDER_EDITING) {
            uint32_t startPos = _waveformSelector.getSelectStart();
            uint32_t endPos = _waveformSelector.getSelectEnd();
            saveBinaryDataFile(_recordedFileName, startPos, endPos);
        }

        if (_navCallback) {
            _navCallback(AppContext::HOME);
            return;
        }
    }

    if (event.buttonId == 2 && event.state == PRESSED) {
        if (currentState == RECORDER_HOME) {
            showRecorderScreen();
        } else if (currentState == RECORDER_RECORDING) {
            stopRecording();
        } else if (currentState == RECORDER_EDITING) {
            String path = getFilePath(_recordedFileName);
            const int WAV_HEADER_SIZE = 44;
            uint32_t startByte =
                _waveformSelector.getSelectStart() * 2 + WAV_HEADER_SIZE;
            uint32_t endByte =
                _waveformSelector.getSelectEnd() * 2 + WAV_HEADER_SIZE;
            _audioResources->playWav1.play(path.c_str(), startByte, endByte,
                                           1.0);

            // refresh();
        }
        return;
    }

    if (event.buttonId == 3 && event.state == PRESSED) {
        if (!event.button1Held && !event.button2Held) {
            if (currentState == RECORDER_EDITING) {
                _waveformSelector.changeSide();
            }
        }
    }

    // Encoder events
    if (event.buttonId == 0 && event.encoderValue != 0) {
        // Button 3 + Encoder = Zoom
        if (event.button3Held && !event.button1Held && !event.button2Held) {
            _waveformSelector.zoom(event.encoderValue);
            _waveformSelector.draw();
            _screen->display();
        }
        // Encoder alone = Update selection
        else if (!event.button1Held && !event.button2Held &&
                 !event.button3Held) {
            if (currentState == RECORDER_EDITING) {
                _waveformSelector.updateSelection(event.encoderValue);
                _waveformSelector.draw();
                _screen->display();
            }
        }
    }
}

void RecorderScreen::showRecorderScreen() {
    currentState = RECORDER_RECORDING;
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "RECORDER");
    _screen->setNormalFont();

    _waveform.clear();
    _waveform.drawWaveform();

    _screen->display();

    startRecording();
}

void RecorderScreen::showEditScreen() {
    currentState = RECORDER_EDITING;
    Serial.println("Showing edit screen for file: " + _recordedFileName);
    _screen->clear();
    _screen->drawStr(0, 10, _recordedFileName.c_str());

    _waveform.clear();
    String path = getFilePath(_recordedFileName);
    _waveform.loadWaveformFile(path.c_str(), 100);
    _waveform.drawCachedWaveform(0, 0);
    _waveformSelector = WaveformSelector(&_waveform);
    _waveformSelector.draw();
    _screen->display();
}

void RecorderScreen::startRecording() {
    // Check if audio resources and WAV writer are available
    if (!_audioResources || !_wavWriter) {
        return;
    }

    _audioResources->unmuteInput();

    _recordedFileName = "";

    // Create RECORDINGS folder if it doesn't exist
    if (!SD.exists("/RECORDINGS")) {
        SD.mkdir("/RECORDINGS");
    }

    String name = gen.generateAudioFilename();

    // Start WAV recording
    String path = getFilePath(name);
    if (_wavWriter->open(path.c_str(), 44100, 1)) {
        _recordedFileName = name;
        _recordingStartTime = millis();
    }
}

void RecorderScreen::updateWaveform() {
    if (!_wavWriter || !_wavWriter->isWriting()) {
        return;
    }

    size_t sampleCount;
    const int16_t* samples = _wavWriter->getAccumulatedBuffer(sampleCount);

    if (sampleCount > 0) {
        // Add all accumulated audio data to waveform
        _waveform.addAudioData(samples, sampleCount);

        // Clear the accumulated buffer for next update cycle
        _wavWriter->clearAccumulatedBuffer();

        // Draw the updated waveform
        _waveform.drawWaveform();
        _screen->display();
    }
}

void RecorderScreen::continueRecording() {
    // Check if WAV writer is available and writing
    if (!_wavWriter || !_wavWriter->isWriting()) {
        return;
    }
    // Update the WAV file
    _wavWriter->update();
}

void RecorderScreen::stopRecording() {
    if (!_wavWriter || !_wavWriter->isWriting()) {
        return;
    }

    _audioResources->muteInput();

    // Close the WAV file
    if (_wavWriter->close()) {
        showEditScreen();
    }
}

void RecorderScreen::updateVolumeBar() {
    if (!_audioResources || currentState != RECORDER_HOME) return;

    if (_audioResources->peak1.available()) {
        float left = _audioResources->peak1.read();
        _volumeBar.setLeftVolume(left);
    }

    if (_audioResources->peak2.available()) {
        float right = _audioResources->peak2.read();
        _volumeBar.setRightVolume(right);
    }

    _volumeBar.drawVolumeBar();
    _screen->display();
}

void RecorderScreen::saveBinaryDataFile(const String& fileName, uint32_t startPos, uint32_t endPos) {
    // Create .bdf file path (filename.wav.bdf)
    String bdfPath = getFilePath(fileName) + ".bdf";

    // Open file for writing
    File bdfFile = SD.open(bdfPath.c_str(), FILE_WRITE);
    if (!bdfFile) {
        Serial.println("Failed to create .bdf file: " + bdfPath);
        return;
    }

    // Write start position (4 bytes, little-endian)
    bdfFile.write((uint8_t)(startPos & 0xFF));
    bdfFile.write((uint8_t)((startPos >> 8) & 0xFF));
    bdfFile.write((uint8_t)((startPos >> 16) & 0xFF));
    bdfFile.write((uint8_t)((startPos >> 24) & 0xFF));

    // Write end position (4 bytes, little-endian)
    bdfFile.write((uint8_t)(endPos & 0xFF));
    bdfFile.write((uint8_t)((endPos >> 8) & 0xFF));
    bdfFile.write((uint8_t)((endPos >> 16) & 0xFF));
    bdfFile.write((uint8_t)((endPos >> 24) & 0xFF));

    bdfFile.close();

    Serial.println("Saved .bdf file: " + bdfPath);
    Serial.print("Start: ");
    Serial.print(startPos);
    Serial.print(", End: ");
    Serial.println(endPos);
}