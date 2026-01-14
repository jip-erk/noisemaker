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
    AudioEffectEnvelope envelope;
    AudioEffectEnvelope envelope1;
    AudioEffectEnvelope envelope2;
    AudioEffectEnvelope envelope3;

    AudioMixer4 mixer4;
    AudioOutputUSB usb;

    AudioRecordQueue queue1;
    AudioMixer4 recordMixer;
    AudioMixer4 recordInputMixer;

    AudioConnection* patchCord[10];
};

#endif