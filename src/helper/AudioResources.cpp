
#include "AudioResources.h"

#include <Arduino.h>
#include <Audio.h>

AudioResources::AudioResources() : patchCord1(audioInput, 0, queue1, 0),
                                   patchCord2(audioInput, 0, peak1, 0),
                                   patchCord3(playWav1, 0, mixer1, 0),
                                   patchCord4(audioInput, 0, mixer1, 1),
                                   patchCord5(mixer1, 0, audioOutput, 0) {
    // Audio connections are initialized in the member initializer list
    // Set mixer gains: playback at full volume, input at 0 (muted during playback)
    mixer1.gain(0, 1.0); // Playback channel
    mixer1.gain(1, 0.0); // Input channel (muted during playback)
}

AudioResources::~AudioResources() {
    // Destructor - no cleanup needed for member objects
}
