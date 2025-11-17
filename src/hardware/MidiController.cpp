#include "MidiController.h"

MidiController::MidiController() : eventCallback(nullptr) {
    // Initialize CC values to 0
    for (int i = 0; i < 128; i++) {
        ccValues[i] = 0;
        noteStates[i] = false;
    }
}

void MidiController::tick() {
    // Check for incoming MIDI messages
    while (usbMIDI.read()) {
        uint8_t type = usbMIDI.getType();
        uint8_t channel = usbMIDI.getChannel();
        uint8_t data1 = usbMIDI.getData1();
        uint8_t data2 = usbMIDI.getData2();

        processMidiMessage(type, data1, data2, channel);
    }
}

void MidiController::processMidiMessage(uint8_t type, uint8_t data1,
                                         uint8_t data2, uint8_t channel) {
    MidiMessageType msgType = MIDI_UNKNOWN;
    int16_t pitch = 0;

    // Convert USB MIDI type to our enum
    switch (type) {
        case usbMIDI.NoteOff:
            msgType = MIDI_NOTE_OFF;
            noteStates[data1] = false;
            break;

        case usbMIDI.NoteOn:
            msgType = MIDI_NOTE_ON;
            // Note: velocity 0 is treated as note off
            if (data2 > 0) {
                noteStates[data1] = true;
            } else {
                noteStates[data1] = false;
                msgType = MIDI_NOTE_OFF;
            }
            break;

        case usbMIDI.ControlChange:
            msgType = MIDI_CONTROL_CHANGE;
            ccValues[data1] = data2;  // Store CC value
            break;

        case usbMIDI.ProgramChange:
            msgType = MIDI_PROGRAM_CHANGE;
            break;

        case usbMIDI.PitchBend:
            msgType = MIDI_PITCH_BEND;
            // Combine data1 (LSB) and data2 (MSB) into 14-bit value
            // Range: 0-16383, center at 8192
            pitch = (data2 << 7) | data1;
            pitch -= 8192;  // Convert to -8192 to +8191
            break;

        default:
            msgType = MIDI_UNKNOWN;
            break;
    }

    // Send event if we have a callback
    if (msgType != MIDI_UNKNOWN && eventCallback) {
        MidiEvent event = createEvent(msgType, channel, data1, data2, pitch);
        eventCallback(event);
    }
}

MidiController::MidiEvent MidiController::createEvent(MidiMessageType type,
                                                       uint8_t channel,
                                                       uint8_t data1,
                                                       uint8_t data2,
                                                       int16_t pitch) {
    MidiEvent event;
    event.type = type;
    event.channel = channel;
    event.data1 = data1;
    event.data2 = data2;
    event.pitch = pitch;
    return event;
}

void MidiController::setEventCallback(MidiEventCallback callback) {
    eventCallback = callback;
}

uint8_t MidiController::getCCValue(uint8_t ccNumber) {
    if (ccNumber < 128) {
        return ccValues[ccNumber];
    }
    return 0;
}

bool MidiController::isNoteHeld(uint8_t note) {
    if (note < 128) {
        return noteStates[note];
    }
    return false;
}
