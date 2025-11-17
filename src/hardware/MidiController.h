#ifndef MidiController_h
#define MidiController_h

#include <Arduino.h>
#include <usb_midi.h>

// MIDI message types
enum MidiMessageType {
    MIDI_NOTE_ON = 0,
    MIDI_NOTE_OFF = 1,
    MIDI_CONTROL_CHANGE = 2,
    MIDI_PROGRAM_CHANGE = 3,
    MIDI_PITCH_BEND = 4,
    MIDI_UNKNOWN = 255
};

class MidiController {
   public:
    MidiController();
    void tick();

    // MIDI Event structure - contains raw MIDI data
    struct MidiEvent {
        MidiMessageType type;
        uint8_t channel;   // 1-16
        uint8_t data1;     // Note number or CC number
        uint8_t data2;     // Velocity or CC value
        int16_t pitch;     // For pitch bend (-8192 to +8191)

        // Convenience methods
        bool isNoteOn() const { return type == MIDI_NOTE_ON && data2 > 0; }
        bool isNoteOff() const { return type == MIDI_NOTE_OFF || (type == MIDI_NOTE_ON && data2 == 0); }
        bool isCC() const { return type == MIDI_CONTROL_CHANGE; }

        uint8_t getNote() const { return data1; }
        uint8_t getVelocity() const { return data2; }
        uint8_t getCCNumber() const { return data1; }
        uint8_t getCCValue() const { return data2; }
    };

    typedef void (*MidiEventCallback)(MidiEvent);
    void setEventCallback(MidiEventCallback callback);

    // Get current CC value for a specific controller
    uint8_t getCCValue(uint8_t ccNumber);

    // Check if a note is currently held
    bool isNoteHeld(uint8_t note);

   private:
    MidiEventCallback eventCallback;

    // Store last CC values (for encoders/knobs)
    uint8_t ccValues[128];

    // Store note states (for pads)
    bool noteStates[128];

    void processMidiMessage(uint8_t type, uint8_t data1, uint8_t data2, uint8_t channel);
    MidiEvent createEvent(MidiMessageType type, uint8_t channel, uint8_t data1, uint8_t data2, int16_t pitch = 0);
};

#endif
