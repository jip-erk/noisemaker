#include "RecorderScreen.h"

RecorderScreen::RecorderScreen() {};

RecorderScreen::RecorderScreen(Screen *screen, AudioResources *audioResources) {
    _screen = screen;
    _audioResources = audioResources;

    _peakArea = _screen->AREA_SCREEN;
    _textArea = _screen->AREA_SCREEN;
}

void RecorderScreen::showRecorderScreen(boolean onScreen) {
    _screen->drawStr(0, 20, "Recorder Screen");
}