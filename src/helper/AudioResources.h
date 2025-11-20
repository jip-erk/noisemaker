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
    void enableLivePassthrough();
    void disableLivePassthrough();

    AudioInputI2S audioInput;
    AudioOutputUSB audioOutput;
    AudioControlSGTL5000 audioShield;
    AudioAnalyzePeak peak1;
    AudioAnalyzePeak peak2;
    AudioPlaySdWavExtended playWav1;
    AudioPlaySdWavExtended playWav2;
    AudioPlaySdWavExtended playWav3;
    AudioPlaySdWavExtended playWav4;

    AudioMixer4 playbackMixer;  // Mixer for 4 WAV players
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
    AudioConnection patchCord13;
    AudioConnection patchCord14;
    AudioConnection patchCord15;
    AudioConnection patchCord16;
};

#endif