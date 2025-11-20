
#include "AudioResources.h"

#include <Arduino.h>
#include <Audio.h>

AudioResources::AudioResources()
    : patchCord1(audioInput, 0, queue1, 0),
      patchCord2(audioInput, 0, peak1, 0),
      patchCord3(audioInput, 1, peak2, 0),
      // Connect 4 WAV players to playbackMixer
      patchCord4(playWav1, 0, playbackMixer, 0),
      patchCord5(playWav2, 0, playbackMixer, 1),
      patchCord6(playWav3, 0, playbackMixer, 2),
      patchCord7(playWav4, 0, playbackMixer, 3),
      // Connect playbackMixer and input to main mixer
      patchCord8(playbackMixer, 0, mixer1, 0),
      patchCord9(audioInput, 0, mixer1, 1),
      // Connect main mixer to USB output
      patchCord10(mixer1, 0, audioOutput, 0),
      patchCord11(mixer1, 0, audioOutput, 1),
      // Add patch cords for recording path
      patchCord12(audioInput, 0, recordInputMixer, 0),
      patchCord13(audioInput, 1, recordInputMixer, 1),
      patchCord14(recordInputMixer, 0, recordMixer, 0),
      patchCord15(recordMixer, 0, queue1, 0),
      patchCord16(recordMixer, 0, peak1, 0) {
    // Initialize mixer gains
    playbackMixer.gain(0, 0.8);
    playbackMixer.gain(1, 0.8);
    playbackMixer.gain(2, 0.8);
    playbackMixer.gain(3, 0.8);
    mixer1.gain(0, 0.8);  // Playback mixer to output
    mixer1.gain(1, 0.0);  // Live input to output (disabled by default)
}

AudioResources::~AudioResources() {
    // Destructor - no cleanup needed for member objects
}

void AudioResources::muteInput() {
    recordInputMixer.gain(0, 0.0);
    recordInputMixer.gain(1, 0.0);
};

void AudioResources::unmuteInput() {
    recordInputMixer.gain(0, 0.6);
    recordInputMixer.gain(1, 0.6);
};

void AudioResources::enableLivePassthrough() {
    mixer1.gain(1, 0.6);  // Enable live mic to output
};

void AudioResources::disableLivePassthrough() {
    mixer1.gain(1, 0.0);  // Disable live mic to output
};
