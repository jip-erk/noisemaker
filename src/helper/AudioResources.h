#ifndef AudioResources_h
#define AudioResources_h

#include <Arduino.h>
#include <Audio.h>

#include "SD.h"
#include "audio-extensions/play_sd_wav_extended.h"

class AudioResources {
   public:
    AudioResources();
    ~AudioResources();

    void muteInput();
    void unmuteInput();

    int activeInput = AUDIO_INPUT_MIC;

    const float maxVolume = 1.0;
    const int maxMicGain = 63;
    const byte maxLineInLevel = 15;

    float currentVolume = 0.5;
    int currentMicGain = 20;

    byte currentLineInLevel = 0;

    AudioInputI2S audioInput;
    AudioOutputUSB audioOutput;
    AudioControlSGTL5000 audioShield;
    AudioAnalyzePeak peak1;
    AudioAnalyzePeak peak2;
    AudioPlaySdWavExtended playWav1;

    AudioMixer4 mixer1;

    AudioRecordQueue queue1;
    AudioMixer4 recordMixer;
    AudioMixer4 recordInputMixer;
    // Audio connections
    AudioConnection patchCord1;
    AudioConnection patchCord2;
    AudioConnection patchCord3;
    AudioConnection patchCord4;
    AudioConnection patchCord5;
    AudioConnection patchCord6;
    AudioConnection patchCord7;
    AudioConnection patchCord8;
    AudioConnection patchCord9;
    AudioConnection patchCord10;
    AudioConnection patchCord11;
    AudioConnection patchCord12;
};

#endif