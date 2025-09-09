#include "RecorderScreen.h"

RecorderScreen::RecorderScreen(Controls *keyboard, Screen *screen,
                               NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
}

void RecorderScreen::refresh() {
    currentState = RECORDER_HOME;
    _screen->clear();
    _screen->drawStr(0, 8, "Recorder");
    _screen->drawStr(0, 35, "Hold enter to start");
    _screen->display();
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
            Serial.println("holding");
            showRecorderScreen();
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

    unsigned long startTime = millis();
    while (millis() - startTime < 2000) {
        continueRecording();
    }
    stopRecording();
}

// recorder

void RecorderScreen::startRecording() {
    if (_sfsio->isRecodingAvailable()) {
        _sfsio->deleteFile(_sfsio->recorderFilename);
    }

    _frec = SD.open(_sfsio->recorderFilename, FILE_WRITE);

    if (_frec) {
        currentState = RECORDER_RECORDING;

        _audioResources->queue1.begin();
    }
}

void RecorderScreen::continueRecording() {
    if (_audioResources->queue1.available() >= 2) {
        //    if (_audioResources->queue1.available() > 40)
        //    Serial.println(_audioResources->queue1.available());

        byte buffer[512];
        memcpy(buffer, _audioResources->queue1.readBuffer(), 256);
        _audioResources->queue1.freeBuffer();
        memcpy(buffer + 256, _audioResources->queue1.readBuffer(), 256);
        _audioResources->queue1.freeBuffer();
        // write all 512 bytes to the SD card
        _frec.write(buffer, 512);
    }
}

void RecorderScreen::stopRecording() {
        _audioResources->queue1.end();
      
        // write last bit of buffer to file..
        while (_audioResources->queue1.available() > 0) {
          _frec.write((byte*)_audioResources->queue1.readBuffer(), 256);
          _audioResources->queue1.freeBuffer();
        }
      
        _frec.close();

        _sfsio->generateWaveFormBufferForSample(3, 0);
      }