#ifndef AudioResources_h
#define AudioResources_h

#include <Arduino.h>
#include <Audio.h>
#include <TeensyVariablePlayback.h>

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
    AudioEffectEnvelope* getEnvelope(int slotIndex);

    AudioInputI2S audioInput;
    AudioControlSGTL5000 audioShield;
    AudioAnalyzePeak peak1;

    AudioPlaySdResmp playSdWav;
    AudioPlaySdResmp playSdWav1;
    AudioPlaySdResmp playSdWav2;
    AudioPlaySdResmp playSdWav3;
    AudioPlaySdResmp playSdWav4;
    AudioPlaySdResmp playSdWav5;
    AudioPlaySdResmp playSdWav6;
    AudioEffectEnvelope envelope;
    AudioEffectEnvelope envelope1;
    AudioEffectEnvelope envelope2;
    AudioEffectEnvelope envelope3;
    AudioEffectEnvelope envelope4;
    AudioEffectEnvelope envelope5;
    AudioEffectEnvelope envelope6;

    AudioPlaySdWavExtended playRecordedWav;

    AudioMixer4 mixer4;
    AudioMixer4 mixer4_2;
    AudioMixer4 finalMixer;
    AudioOutputUSB usb;

    AudioRecordQueue queue1;
    AudioMixer4 recordMixer;
    AudioMixer4 recordInputMixer;

    AudioConnection* patchCord[40];
};

#endif