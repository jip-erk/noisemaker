#ifndef VolumeBar_h
#define VolumeBar_h

#include <Arduino.h>

#include "../../Screen.h"

class VolumeBar {
   public:
    VolumeBar();
    VolumeBar(Screen *screen);
    VolumeBar(Screen *screen, int x, int y, int width, int height);
    void drawVolumeBar();
    void setLeftVolume(float left);
    void setRightVolume(float right);

   private:
    void drawBar(int x, int y, float value, float peakValue);

    Screen *_screen;
    int _x = 0;
    int _y = 0;
    int _width = 100;
    int _height = 10;
    float _leftVolume = 0.0;
    float _rightVolume = 0.0;

    // Peak hold values
    float _leftPeak = 0.0;
    float _rightPeak = 0.0;
    unsigned long _leftPeakTime = 0;
    unsigned long _rightPeakTime = 0;
    static const unsigned long PEAK_HOLD_TIME_MS = 1000;  // 1 second
};

#endif
