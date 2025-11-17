
#include "AudioResources.h"

#include <Arduino.h>
#include <Audio.h>

AudioResources::AudioResources()
    : patchCord1(audioInput, 0, queue1, 0),
      patchCord2(audioInput, 0, peak1, 0),
      patchCord3(audioInput, 1, peak2, 0),
      patchCord4(playWav1, 0, mixer1, 0),
      patchCord5(audioInput, 0, mixer1, 1),
      patchCord6(mixer1, 0, audioOutput, 0),
      patchCord7(mixer1, 0, audioOutput, 1),
      // Add patch cords for recording path
      patchCord8(audioInput, 0, recordInputMixer, 0),
      patchCord9(audioInput, 1, recordInputMixer, 1),
      patchCord10(recordInputMixer, 0, recordMixer, 0),
      patchCord11(recordMixer, 0, queue1, 0),
      patchCord12(recordMixer, 0, peak1, 0) {}

AudioResources::~AudioResources() {
    // Destructor - no cleanup needed for member objects
}

void AudioResources::muteInput() {
    recordInputMixer.gain(0, 0.0);
    recordInputMixer.gain(1, 0.0);
    mixer1.gain(1, 0.0);
};

void AudioResources::unmuteInput() {
    recordInputMixer.gain(0, 0.6);
    recordInputMixer.gain(1, 0.6);
    mixer1.gain(1, 0.6);
};
