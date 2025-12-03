#ifndef RecorderScreen_h
#define RecorderScreen_h

#include <Arduino.h>

#include "../../hardware/Controls.h"
#include "../Screen.h"
#include "components/VolumeBar.h"
#include "components/waveform/Waveform.h"
#include "components/waveform/WaveformSelector.hpp"

class RecorderScreen {
   public:
    RecorderScreen();
    RecorderScreen(Screen* screen);
    ~RecorderScreen();

    // UI Display methods
    void refresh();
    void showRecordingScreen();
    void showEditScreen(const String& fileName, const String& filePath);
    void drawVolumeBar();
    void drawWaveform();

    // Volume control
    void setLeftVolume(float left);
    void setRightVolume(float right);

    // Waveform manipulation
    void addAudioData(const int16_t* samples, size_t sampleCount);
    void changeSide();
    void updateSelection(int encoderValue);
    void zoom(int encoderValue);

    // Getter methods
    uint32_t getSelectStart() const;
    uint32_t getSelectEnd() const;

   private:
    Screen* _screen;
    VolumeBar _volumeBar;
    Waveform _waveform;
    WaveformSelector _waveformSelector;
};

#endif
