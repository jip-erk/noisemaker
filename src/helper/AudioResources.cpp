
#include "AudioResources.h"

#include <Arduino.h>
#include <Audio.h>

AudioResources::AudioResources() : patchCord1(audioInput, 0, queue1, 0),
                                   patchCord2(audioInput, 0, peak1, 0) {
    // Audio connections are initialized in the member initializer list
}

AudioResources::~AudioResources() {
    // Destructor - no cleanup needed for member objects
}
