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
    _screen->drawStr(0, 35, "Hold enter to start");
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

    if (event.buttonId == 2 && event.state == LONG_PRESSED) {
        if (currentState == RECORDER_HOME) {
            currentState = RECORDER_RECORDING;
            showRecorderScreen();
        } else if (currentState == RECORDER_RECORDING) {
            // Stop recording on long press while recording
            stopRecording();
            refresh();
        }
        return;
    }

    if (currentState == RECORDER_HOME) refresh();
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