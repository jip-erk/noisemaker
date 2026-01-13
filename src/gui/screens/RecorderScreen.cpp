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

RecorderScreen::~RecorderScreen() {}

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
    drawRecordingHeader();

    _waveform.clear();
    _waveform.drawWaveform();

    _screen->display();
}

void RecorderScreen::showEditScreen(const String& fileName,
                                    const String& filePath) {
    _screen->clear();
    _screen->drawStr(0, 10, fileName.c_str());

    _waveform.clear();
    _waveform.loadWaveformFile(filePath.c_str(), 100);
    _waveform.drawCachedWaveform(0, 0);
    _waveformSelector = WaveformSelector(&_waveform);
    _waveformSelector.draw();
    drawSelectionIndicator();
    _screen->display();
}

void RecorderScreen::drawVolumeBar() { _volumeBar.drawVolumeBar(); }

void RecorderScreen::drawWaveform() { _waveform.drawWaveform(); }

void RecorderScreen::setVolume(float volume) { _volumeBar.setVolume(volume); }

void RecorderScreen::addAudioData(const int16_t* samples, size_t sampleCount) {
    _waveform.addAudioData(samples, sampleCount);
}

void RecorderScreen::changeSide() {
    _waveformSelector.changeSide();
    _waveformSelector.draw();
    drawSelectionIndicator();
}

void RecorderScreen::updateSelection(int encoderValue) {
    _waveformSelector.updateSelection(encoderValue);
    _waveformSelector.draw();
    drawSelectionIndicator();
}

void RecorderScreen::zoom(int encoderValue) {
    _waveformSelector.zoom(encoderValue);
    _waveformSelector.draw();
    drawSelectionIndicator();
}

void RecorderScreen::pan(int encoderValue) {
    _waveformSelector.pan(encoderValue);
    _waveformSelector.draw();
    drawSelectionIndicator();
}

uint32_t RecorderScreen::getSelectStart() const {
    return _waveformSelector.getSelectStart();
}

uint32_t RecorderScreen::getSelectEnd() const {
    return _waveformSelector.getSelectEnd();
}

bool RecorderScreen::isSelectingLeft() const {
    return _waveformSelector.isSelectingLeft();
}

void RecorderScreen::setRecordingTime(unsigned long elapsedMs) {
    _recordingTimeMs = elapsedMs;
}

void RecorderScreen::drawRecordingHeader() {
    _screen->setHeaderFont();

    // Draw header with timer
    unsigned long seconds = _recordingTimeMs / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long secs = seconds % 60;

    char timeBuffer[20];
    snprintf(timeBuffer, sizeof(timeBuffer), "REC %02lu:%02lu     ", minutes, secs);

    // Clear and draw timer - pad with spaces to overwrite old text
    _screen->drawStr(0, 10, "            ");  // Clear left side first
    _screen->drawStr(30, 10, timeBuffer);

    _screen->setNormalFont();
}

void RecorderScreen::drawSelectionIndicator() {
    _screen->setNormalFont();
    // Clear the area first
    _screen->drawStr(100, 62, "     ");
    // Draw the new indicator
    const char* indicator = _waveformSelector.isSelectingLeft() ? "START" : "END";
    _screen->drawStr(100, 62, indicator);
}