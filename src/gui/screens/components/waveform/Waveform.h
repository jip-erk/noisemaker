#ifndef Waveform_h
#define Waveform_h

#include <SD.h>

#include "../../../Screen.h"
#define MAX_WAVEFORM_POINTS 122  // Width minus border

class Waveform {
   public:
    Waveform();
    Waveform(Screen* screen);
    Waveform(Screen* screen, int x, int y, int width, int height);
    ~Waveform();

    void setPosition(int x, int y);
    void setSize(int width, int height);
    void clear();
    void addAudioData(const int16_t* audioBuffer, int bufferSize);

    bool loadWaveformFile(const char* fileName, int maxMemoryKB = 100);
    void drawCachedWaveform(int startSample, int endSample = 0);

    void drawSelection(int selectStart, int selectEnd, int startSample,
                       int endSample);
    void drawWaveform();

    void drawWaveform(int startSample, int endSample);
    int getTotalSamples() const { return _totalSamples; }

   private:
    Screen* _screen;
    int _x = 0, _y = 0;
    int _width = 128, _height = 47;
    int16_t* _samples = nullptr;
    int _totalSamples = 0;

    int16_t* _minCache = nullptr;
    int16_t* _maxCache = nullptr;
    int _cacheSize = 0;
    int _samplesPerCachePoint = 1;

    void freeCacheMemory();

    int _waveformData[MAX_WAVEFORM_POINTS];
    int _waveformLength = 0;
    int _writeIndex = 0;
};

#endif