
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
    patchCord[pci++] = new AudioConnection(playSdWav4, 0, envelope4, 0);
    patchCord[pci++] = new AudioConnection(playSdWav5, 0, envelope5, 0);
    patchCord[pci++] = new AudioConnection(playSdWav6, 0, envelope6, 0);

    // Connect envelopes to mixers
    // Mixer 1 (channels 0-3)
    patchCord[pci++] = new AudioConnection(envelope, 0, mixer4, 0);
    patchCord[pci++] = new AudioConnection(envelope1, 0, mixer4, 1);
    patchCord[pci++] = new AudioConnection(envelope2, 0, mixer4, 2);
    patchCord[pci++] = new AudioConnection(envelope3, 0, mixer4, 3);

    // Mixer 2 (channels 4-6)
    patchCord[pci++] = new AudioConnection(envelope4, 0, mixer4_2, 0);
    patchCord[pci++] = new AudioConnection(envelope5, 0, mixer4_2, 1);
    patchCord[pci++] = new AudioConnection(envelope6, 0, mixer4_2, 2);

    // Mix mixer4 (tracks 0-3), mixer4_2 (tracks 4-6) and playRecordedWav into
    // finalMixer
    patchCord[pci++] = new AudioConnection(mixer4, 0, finalMixer, 0);
    patchCord[pci++] = new AudioConnection(mixer4_2, 0, finalMixer, 1);
    patchCord[pci++] = new AudioConnection(playRecordedWav, 0, finalMixer, 2);

    // Connect finalMixer to USB output
    patchCord[pci++] = new AudioConnection(finalMixer, 0, usb, 0);
    patchCord[pci++] = new AudioConnection(finalMixer, 0, usb, 1);

    // Set default envelope parameters for all envelopes
    AudioEffectEnvelope* envelopes[] = {&envelope,  &envelope1, &envelope2,
                                        &envelope3, &envelope4, &envelope5,
                                        &envelope6};
    for (int i = 0; i < 7; i++) {
        envelopes[i]->attack(0);
        envelopes[i]->hold(0);
        envelopes[i]->delay(0);
        envelopes[i]->decay(10);
        envelopes[i]->sustain(1);
        envelopes[i]->release(0);
    }

    // Set mixer gains to prevent clipping with 4 inputs
    // Reduced from 0.5 to 0.25 to prevent summation clipping
    mixer4.gain(0, .25);
    mixer4.gain(1, .25);
    mixer4.gain(2, .25);
    mixer4.gain(3, .25);

    // Mixer 2 settings
    mixer4_2.gain(0, .25);
    mixer4_2.gain(1, .25);
    mixer4_2.gain(2, .25);
    mixer4_2.gain(3, 0);  // Unused

    // Final mixer settings
    finalMixer.gain(0, 1.0);  // Tracks 0-3
    finalMixer.gain(1, 1.0);  // Tracks 4-6
    finalMixer.gain(2, 0.0);  // Recorded wav preview (muted by default)

    // Initialize variable playback interpolation
    playSdWav.enableInterpolation(true);
    playSdWav1.enableInterpolation(true);
    playSdWav2.enableInterpolation(true);
    playSdWav3.enableInterpolation(true);
    playSdWav4.enableInterpolation(true);
    playSdWav5.enableInterpolation(true);
    playSdWav6.enableInterpolation(true);
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
    finalMixer.gain(2, 1.0);  // Enable recorded wav preview
}

void AudioResources::disableLivePassthrough() {
    finalMixer.gain(2, 0.0);  // Disable recorded wav preview
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
        case 4:
            return &envelope4;
        case 5:
            return &envelope5;
        case 6:
            return &envelope6;
        default:
            return nullptr;
    }
}
