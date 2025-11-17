#include "VolumeBar.h"

VolumeBar::VolumeBar() : _screen(nullptr) {}

VolumeBar::VolumeBar(Screen *screen) : _screen(screen) {}

VolumeBar::VolumeBar(Screen *screen, int x, int y, int width, int height)
    : _screen(screen), _x(x), _y(y), _width(width), _height(height) {}

void VolumeBar::setLeftVolume(float left) {
    _leftVolume = left;

    unsigned long currentTime = millis();
    // Update peak if new value is higher or if hold time expired
    if (left > _leftPeak || (currentTime - _leftPeakTime) >= PEAK_HOLD_TIME_MS) {
        _leftPeak = left;
        _leftPeakTime = currentTime;
    }
}

void VolumeBar::setRightVolume(float right) {
    _rightVolume = right;

    unsigned long currentTime = millis();
    // Update peak if new value is higher or if hold time expired
    if (right > _rightPeak || (currentTime - _rightPeakTime) >= PEAK_HOLD_TIME_MS) {
        _rightPeak = right;
        _rightPeakTime = currentTime;
    }
}

void VolumeBar::drawVolumeBar() {
    if (!_screen) return;

    // Draw solid white rounded background
    _screen->getDisplay()->setDrawColor(1);
    _screen->getDisplay()->drawRBox(_x, _y, _width, _height, 2);

    drawBar(_x + 2, _y + 1, _leftVolume, _leftPeak);
    drawBar(_x + 2, _y + 6, _rightVolume, _rightPeak);

    _screen->getDisplay()->setDrawColor(1);
}

void VolumeBar::drawBar(int x, int y, float value, float peakValue) {
    if (!_screen) return;

    int barWidth = _width - 4;
    int barHeight = 3;

    int currentVolumeWidth = (int)(value * barWidth);
    if (currentVolumeWidth > barWidth) currentVolumeWidth = barWidth;

    // Draw current volume fill (black)
    if (currentVolumeWidth > 0) {
        _screen->getDisplay()->setDrawColor(0);
        _screen->getDisplay()->drawBox(x, y, currentVolumeWidth, barHeight);
    }

    // Draw peak hold line (black vertical line)
    int peakPosition = (int)(peakValue * barWidth);
    if (peakPosition > barWidth) peakPosition = barWidth;
    if (peakPosition > 0) {
        _screen->getDisplay()->setDrawColor(0);
        _screen->getDisplay()->drawVLine(x + peakPosition, y, barHeight);
    }
}