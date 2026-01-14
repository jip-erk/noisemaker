
#include "AudioResources.h"

#include <Arduino.h>
#include <Audio.h>

AudioResources::AudioResources() {
    // Audio routing: 4 sample players -> 4 envelopes -> mixer4 -> USB output
    int pci = 0;  // Counter for patchcord allocation

    patchCord[pci++] = new AudioConnection(audioInput, queue1);
    patchCord[pci++] = new AudioConnection(audioInput, peak1);

    // Connect sample players to envelopes
    patchCord[pci++] = new AudioConnection(playSdWav, 0, envelope, 0);
    patchCord[pci++] = new AudioConnection(playSdWav1, 0, envelope1, 0);
    patchCord[pci++] = new AudioConnection(playSdWav2, 0, envelope2, 0);
    patchCord[pci++] = new AudioConnection(playSdWav3, 0, envelope3, 0);
    patchCord[pci++] = new AudioConnection(envelope, 0, mixer4, 0);
    patchCord[pci++] = new AudioConnection(envelope1, 0, mixer4, 1);
    patchCord[pci++] = new AudioConnection(envelope2, 0, mixer4, 2);
    patchCord[pci++] = new AudioConnection(envelope3, 0, mixer4, 3);
    // Connect mixer to USB output
    patchCord[pci++] = new AudioConnection(mixer4, 0, usb, 0);

    // Set default envelope parameters for all envelopes
    AudioEffectEnvelope* envelopes[] = {&envelope, &envelope1, &envelope2,
                                        &envelope3};
    for (int i = 0; i < 4; i++) {
        envelopes[i]->attack(0);
        envelopes[i]->hold(0);
        envelopes[i]->delay(0);
        envelopes[i]->decay(10);
        envelopes[i]->sustain(1);
        envelopes[i]->release(0);
    }

    // Set mixer gains to prevent clipping with 4 inputs
    mixer4.gain(0, .5);
    mixer4.gain(1, .5);
    mixer4.gain(2, .5);
    mixer4.gain(3, .5);

    // Initialize variable playback interpolation
    playSdWav.enableInterpolation(true);
    playSdWav1.enableInterpolation(true);
    playSdWav2.enableInterpolation(true);
    playSdWav3.enableInterpolation(true);
}

AudioResources::~AudioResources() {
    // Destructor - no cleanup needed for member objects
}

void AudioResources::muteInput() {
    recordInputMixer.gain(0, 0.0);
    recordInputMixer.gain(1, 0.0);
}

void AudioResources::unmuteInput() {
    recordInputMixer.gain(0, 0.6);
    recordInputMixer.gain(1, 0.6);
}

void AudioResources::enableLivePassthrough() {
    // mixer4.gain(2, 0.6);  // Enable live mic to output
}

void AudioResources::disableLivePassthrough() {
    // mixer4.gain(2, 0.0);  // Disable live mic to output
}

AudioEffectEnvelope* AudioResources::getEnvelope(int slotIndex) {
    switch (slotIndex) {
        case 0:
            return &envelope;
        case 1:
            return &envelope1;
        case 2:
            return &envelope2;
        case 3:
            return &envelope3;
        default:
            return nullptr;
    }
}
