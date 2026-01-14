#include "Recorder.h"

// Timer interval constants (in microseconds)
static const long VOLUME_UPDATE_INTERVAL_US = 70000;     // ~14 Hz
static const long WAVEFORM_UPDATE_INTERVAL_US = 500000;  // 2 Hz
static const long DEFAULT_TICK_INTERVAL_US = 1000000;    // 1 Hz

Recorder::Recorder(Controls* keyboard, Screen* screen,
                   NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
    _audioResources = nullptr;
    _wavWriter = nullptr;
    _recorderScreen = RecorderScreen(screen);
}

Recorder::~Recorder() {
    if (_wavWriter) {
        delete _wavWriter;
        _wavWriter = nullptr;
    }
}

void Recorder::refresh() {
    currentState = RECORDER_HOME;
    _keyboard->triggerLedForButton(1, false);
    findHighestRecordingNumber();
    _recorderScreen.refresh();
}

long Recorder::receiveTimerTick() {
    if (currentState == RECORDER_HOME) {
        updateVolumeBar();
        return VOLUME_UPDATE_INTERVAL_US;
    } else if (currentState == RECORDER_RECORDING) {
        // Update recording time display
        unsigned long elapsedTime = millis() - _recordingStartTime;
        _recorderScreen.setRecordingTime(elapsedTime);

        // Blink button 1 LED (500ms on, 500ms off - synchronized with update
        // interval)
        bool ledState = (elapsedTime % 1000) < 500;
        _keyboard->triggerLedForButton(1, ledState);

        updateWaveform();
        return WAVEFORM_UPDATE_INTERVAL_US;
    }

    return DEFAULT_TICK_INTERVAL_US;
}

void Recorder::setAudioResources(AudioResources* audioResources) {
    _audioResources = audioResources;
    // Create WavFileWriter with the audio queue
    _wavWriter = new WavFileWriter(_audioResources->queue1);
}

void Recorder::handleEvent(Controls::ButtonEvent event) {
    if (event.buttonId == 1 && event.state == PRESSED) {
        // Stop recording
        if (currentState == RECORDER_RECORDING) {
            stopRecording();
            return;
        }

        // In editing mode, button 1 is shift modifier only - no solo action
        // (Button 1 + Button 4 handled in button 4 section)
        if (currentState == RECORDER_EDITING) {
            return;
        }

        if (_navCallback) {
            _navCallback(AppContext::HOME);
            return;
        }
    }

    if (event.buttonId == 2 && event.state == PRESSED) {
        if (currentState == RECORDER_HOME) {
            if (_navCallback) {
                _navCallback(AppContext::HOME);
                return;
            }
        }
        return;
    }

    if (event.buttonId == 4 && event.state == PRESSED) {
        if (currentState == RECORDER_HOME) {
            showRecorderScreen();
        } else if (currentState == RECORDER_RECORDING) {
            stopRecording();
        } else if (currentState == RECORDER_EDITING) {
            // Button 1 held + Button 4 = Go back to home
            if (event.button1Held) {
                if (_navCallback) {
                    _navCallback(AppContext::HOME);
                    return;
                }
            } else {
                // Button 4 alone = Play sample
                String path = getFilePath(_recordedFileName);
                const int WAV_HEADER_SIZE = 44;
                uint32_t startByte =
                    _recorderScreen.getSelectStart() * 2 + WAV_HEADER_SIZE;
                uint32_t endByte =
                    _recorderScreen.getSelectEnd() * 2 + WAV_HEADER_SIZE;
                // _audioResources->playWav1.play(path.c_str(), startByte,
                // endByte,
                //                                1.0);
            }
        }
        return;
    }

    // Button 5 = Change sides or trim with button 1
    if (event.buttonId == 5 && event.state == PRESSED) {
        if (currentState == RECORDER_EDITING) {
            // Button 1 held + Button 5 = Trim audio file to selection
            if (event.button1Held) {
                uint32_t startPos = _recorderScreen.getSelectStart();
                uint32_t endPos = _recorderScreen.getSelectEnd();
                trimAudioFile(_recordedFileName, startPos, endPos);

                // Show save confirmation
                _screen->clear();
                _screen->drawStr(0, 10, _recordedFileName.c_str());
                _screen->drawStr(0, 30, "Trimmed!");
                _screen->display();
                delay(500);

                // Redraw edit screen
                showEditScreen();
            } else {
                // Button 5 alone = Change sides
                _recorderScreen.changeSide();
            }
        }
        return;
    }

    // Encoder events
    if (event.buttonId == 0 && event.encoderValue != 0) {
        if (currentState == RECORDER_EDITING) {
            // Button 3 held + Encoder = Zoom
            if (event.button3Held && !event.button1Held && !event.button2Held) {
                _recorderScreen.zoom(event.encoderValue);
                _screen->display();
            }
            // Button 2 held + Encoder = Pan
            else if (event.button2Held && !event.button1Held &&
                     !event.button3Held) {
                _recorderScreen.pan(event.encoderValue);
                _screen->display();
            }
            // Encoder alone = Update selection
            else if (!event.button1Held && !event.button2Held &&
                     !event.button3Held) {
                _recorderScreen.updateSelection(event.encoderValue);
                _screen->display();
            }
        }
    }
}

void Recorder::showRecorderScreen() {
    currentState = RECORDER_RECORDING;
    _recorderScreen.showRecordingScreen();
    startRecording();
}

void Recorder::showEditScreen() {
    currentState = RECORDER_EDITING;
    Serial.println("Showing edit screen for file: " + _recordedFileName);
    _recorderScreen.showEditScreen(_recordedFileName,
                                   getFilePath(_recordedFileName));
}

void Recorder::startRecording() {
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

    // Create filename with unique number
    _recordingNumber++;
    char nameBuffer[16];
    snprintf(nameBuffer, sizeof(nameBuffer), "REC_%04u", _recordingNumber);
    String name = String(nameBuffer);

    // Start WAV recording
    String path = getFilePath(name);
    if (_wavWriter->open(path.c_str(), 44100, 1)) {
        _recordedFileName = name;
        _recordingStartTime = millis();
    }
}

void Recorder::updateWaveform() {
    if (!_wavWriter || !_wavWriter->isWriting()) {
        return;
    }

    size_t sampleCount;
    const int16_t* samples = _wavWriter->getAccumulatedBuffer(sampleCount);

    if (sampleCount > 0) {
        // Add all accumulated audio data to waveform
        _recorderScreen.addAudioData(samples, sampleCount);

        // Clear the accumulated buffer for next update cycle
        _wavWriter->clearAccumulatedBuffer();

        // Draw the updated waveform
        _recorderScreen.drawWaveform();
    }

    // Always redraw header to update timer and button blink
    _recorderScreen.drawRecordingHeader();
    _screen->display();
}

void Recorder::continueRecording() {
    // Check if WAV writer is available and writing
    if (!_wavWriter || !_wavWriter->isWriting()) {
        return;
    }
    // Update the WAV file
    _wavWriter->update();
}

void Recorder::stopRecording() {
    if (!_wavWriter || !_wavWriter->isWriting()) {
        return;
    }

    _audioResources->muteInput();
    _keyboard->triggerLedForButton(1, false);

    // Close the WAV file
    if (_wavWriter->close()) {
        showEditScreen();
    }
}

void Recorder::updateVolumeBar() {
    if (!_audioResources || currentState != RECORDER_HOME) return;

    if (_audioResources->peak1.available()) {
        float volume = _audioResources->peak1.read();
        _recorderScreen.setVolume(volume);
    }

    _recorderScreen.drawVolumeBar();
    _screen->display();
}

void Recorder::trimAudioFile(const String& fileName, uint32_t startPos,
                             uint32_t endPos) {
    String originalPath = getFilePath(fileName);
    String tempPath = getFilePath(fileName) + ".tmp";

    // Clamp positions
    if (startPos >= endPos) {
        Serial.println("Invalid trim range");
        return;
    }

    // Open original file
    File originalFile = SD.open(originalPath.c_str(), FILE_READ);
    if (!originalFile) {
        Serial.println("Failed to open original file: " + originalPath);
        return;
    }

    // Read WAV header (44 bytes)
    uint8_t header[44];
    int bytesRead = originalFile.read(header, 44);
    if (bytesRead < 44) {
        Serial.println("Invalid WAV file - header too short");
        originalFile.close();
        return;
    }

    // Calculate new audio data size
    uint32_t numSamples = endPos - startPos;
    uint32_t numBytes = numSamples * 2;  // 16-bit samples = 2 bytes each

    // Update WAV header with new size
    // File size at bytes 4-7 (total file size - 8)
    uint32_t newFileSize = 36 + numBytes;
    header[4] = (newFileSize) & 0xFF;
    header[5] = (newFileSize >> 8) & 0xFF;
    header[6] = (newFileSize >> 16) & 0xFF;
    header[7] = (newFileSize >> 24) & 0xFF;

    // Data subchunk size at bytes 40-43
    header[40] = (numBytes) & 0xFF;
    header[41] = (numBytes >> 8) & 0xFF;
    header[42] = (numBytes >> 16) & 0xFF;
    header[43] = (numBytes >> 24) & 0xFF;

    // Seek to start of audio data (startPos samples after header)
    uint32_t startByte = startPos * 2 + 44;
    originalFile.seek(startByte);

    // Create temporary file
    File tempFile = SD.open(tempPath.c_str(), FILE_WRITE);
    if (!tempFile) {
        Serial.println("Failed to create temp file: " + tempPath);
        originalFile.close();
        return;
    }

    // Write header to temp file
    tempFile.write(header, 44);

    // Copy audio data in chunks
    const int CHUNK_SIZE = 512;
    uint8_t buffer[CHUNK_SIZE];
    uint32_t bytesRemaining = numBytes;

    while (bytesRemaining > 0) {
        int toRead =
            (bytesRemaining < CHUNK_SIZE) ? bytesRemaining : CHUNK_SIZE;
        int bytesActuallyRead = originalFile.read(buffer, toRead);

        if (bytesActuallyRead <= 0) {
            Serial.println("Error reading audio data");
            break;
        }

        tempFile.write(buffer, bytesActuallyRead);
        bytesRemaining -= bytesActuallyRead;
    }

    originalFile.close();
    tempFile.close();

    // Delete original and rename temp to original
    SD.remove(originalPath.c_str());
    SD.rename(tempPath.c_str(), originalPath.c_str());

    Serial.println("Trimmed audio file: " + originalPath);
    Serial.print("Samples: ");
    Serial.println((unsigned long)numSamples);
}

void Recorder::findHighestRecordingNumber() {
    _recordingNumber = 0;

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

    while (true) {
        File entry = recordingsDir.openNextFile();
        if (!entry) break;

        String filename = entry.name();
        if (!entry.isDirectory() && (filename.endsWith(".wav") || filename.endsWith(".WAV"))) {
            // Parse filename: REC_XXXX.wav
            if (filename.startsWith("REC_")) {
                String numberStr = filename.substring(4, 8);  // Extract "XXXX" from "REC_XXXX"
                uint32_t fileNumber = numberStr.toInt();
                if (fileNumber > _recordingNumber) {
                    _recordingNumber = fileNumber;
                }
            }
        }
        entry.close();
    }

    recordingsDir.close();
    Serial.print("Found highest recording number: ");
    Serial.println(_recordingNumber);
}
