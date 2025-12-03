#include "RecorderScreen.h"

RecorderScreen::RecorderScreen() {
    _screen = nullptr;
    _volumeBar = VolumeBar(nullptr, 68, 0, 60, 10);
    _waveform = Waveform(nullptr, 0, 15, 128, 47);
    _waveformSelector = WaveformSelector(&_waveform);
}

RecorderScreen::RecorderScreen(Screen* screen) {
    _screen = screen;
    _volumeBar = VolumeBar(screen, 68, 0, 60, 10);
    _waveform = Waveform(screen, 0, 15, 128, 47);
    _waveformSelector = WaveformSelector(&_waveform);
}

RecorderScreen::~RecorderScreen() {
}

void RecorderScreen::refresh() {
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "RECORDER");
    _screen->setNormalFont();
    _screen->drawStr(0, 20, "Click to start");
    _volumeBar.drawVolumeBar();
    _screen->display();
}

void RecorderScreen::showRecordingScreen() {
    _screen->clear();
    _screen->setHeaderFont();
    _screen->drawStr(0, 10, "RECORDER");
    _screen->setNormalFont();

    _waveform.clear();
    _waveform.drawWaveform();

    _screen->display();
}

void RecorderScreen::showEditScreen(const String& fileName, const String& filePath) {
    _screen->clear();
    _screen->drawStr(0, 10, fileName.c_str());

    _waveform.clear();
    _waveform.loadWaveformFile(filePath.c_str(), 100);
    _waveform.drawCachedWaveform(0, 0);
    _waveformSelector = WaveformSelector(&_waveform);
    _waveformSelector.draw();
    _screen->display();
}

void RecorderScreen::drawVolumeBar() {
    _volumeBar.drawVolumeBar();
}

void RecorderScreen::drawWaveform() {
    _waveform.drawWaveform();
}

void RecorderScreen::setLeftVolume(float left) {
    _volumeBar.setLeftVolume(left);
}

void RecorderScreen::setRightVolume(float right) {
    _volumeBar.setRightVolume(right);
}

void RecorderScreen::addAudioData(const int16_t* samples, size_t sampleCount) {
    _waveform.addAudioData(samples, sampleCount);
}

void RecorderScreen::changeSide() {
    _waveformSelector.changeSide();
}

void RecorderScreen::updateSelection(int encoderValue) {
    _waveformSelector.updateSelection(encoderValue);
    _waveformSelector.draw();
}

void RecorderScreen::zoom(int encoderValue) {
    _waveformSelector.zoom(encoderValue);
    _waveformSelector.draw();
}

uint32_t RecorderScreen::getSelectStart() const {
    return _waveformSelector.getSelectStart();
}

uint32_t RecorderScreen::getSelectEnd() const {
    return _waveformSelector.getSelectEnd();
}