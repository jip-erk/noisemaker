#ifndef AudioResources_h
#define AudioResources_h

#include <Arduino.h>
#include <Audio.h>

class AudioResources {
   public:
    AudioResources();
    ~AudioResources();

    int activeInput = AUDIO_INPUT_MIC;

    const float maxVolume = 1.0;
    const int maxMicGain = 63;
    const byte maxLineInLevel = 15;

    float currentVolume = 0.5;
    int currentMicGain = 20;

    byte currentLineInLevel = 0;

    AudioInputI2S audioInput;
    AudioOutputI2S audioOutput;
    AudioControlSGTL5000 audioShield;
    AudioAnalyzePeak peak1;
    AudioRecordQueue queue1;
    AudioPlaySdWav playWav1;
    AudioMixer4 mixer1;
    
    // Audio connections
    AudioConnection patchCord1;
    AudioConnection patchCord2;
    AudioConnection patchCord3;
    AudioConnection patchCord4;
    AudioConnection patchCord5;
};

#endif