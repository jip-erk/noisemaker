#include "VolumeBar.h"

VolumeBar::VolumeBar() : _screen(nullptr) {}

VolumeBar::VolumeBar(Screen *screen) : _screen(screen) {}

VolumeBar::VolumeBar(Screen *screen, int x, int y, int width, int height)
    : _screen(screen), _x(x), _y(y), _width(width), _height(height) {}

void VolumeBar::setLeftVolume(float left) { _leftVolume = left; }
void VolumeBar::setRightVolume(float right) { _rightVolume = right; }

void VolumeBar::drawVolumeBar() {
    if (!_screen) return;

    // Draw solid white rounded background
    _screen->getDisplay()->setDrawColor(1);
    _screen->getDisplay()->drawRBox(_x, _y, _width, _height, 2);

    drawBar(_x + 2, _y + 1, _leftVolume);
    drawBar(_x + 2, _y + 6, _rightVolume);

    _screen->getDisplay()->setDrawColor(1);
}

void VolumeBar::drawBar(int x, int y, float value) {
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
}