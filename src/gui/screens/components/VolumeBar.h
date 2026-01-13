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
    void setVolume(float volume);

   private:
    void drawBar(int x, int y, float value);

    Screen *_screen;
    int _x = 0;
    int _y = 0;
    int _width = 100;
    int _height = 10;
    float _volume = 0.0;
};

#endif
