#ifndef MidiMappings_h
#define MidiMappings_h

#include <Arduino.h>

#include "../main.h"
#include "Controls.h"
#include "MidiController.h"

// Action types that MIDI can trigger
enum MidiActionType {
    MIDI_ACTION_NONE = 0,              // No action
    MIDI_ACTION_ENCODER = 1,           // Simulate encoder rotation
    MIDI_ACTION_BUTTON = 2,            // Simulate button press/release
    MIDI_ACTION_NAVIGATION_UP = 3,     // Navigate menu up
    MIDI_ACTION_NAVIGATION_DOWN = 4,   // Navigate menu down
    MIDI_ACTION_NAVIGATION_SELECT = 5, // Select menu item
    MIDI_ACTION_NAVIGATION_BACK = 6,   // Go back / cancel
    MIDI_ACTION_TRANSPORT_RECORD = 7,  // Start/stop recording
    MIDI_ACTION_TRANSPORT_PLAY = 8,    // Play
    MIDI_ACTION_TRANSPORT_STOP = 9,    // Stop
    // Recorder editing actions
    MIDI_ACTION_RECORDER_SET_START = 10,     // Set start position (knob 5)
    MIDI_ACTION_RECORDER_SET_END = 11,       // Set end position (knob 6)
    MIDI_ACTION_RECORDER_ZOOM = 12,          // Zoom waveform (knob 7)
    MIDI_ACTION_RECORDER_UPDATE_SELECTION = 13, // Update selection (knob 8)
    MIDI_ACTION_CUSTOM = 100               // Custom context-specific actions
};

// Structure to define what action a MIDI message triggers
struct MidiAction {
    MidiActionType actionType;
    uint8_t targetId;  // Button ID or parameter ID depending on action type
    int8_t value;      // Value for the action (e.g., encoder direction)

    MidiAction() : actionType(MIDI_ACTION_NONE), targetId(0), value(0) {}
    MidiAction(MidiActionType type, uint8_t target = 0, int8_t val = 0)
        : actionType(type), targetId(target), value(val) {}
};

class MidiMappings {
   public:
    MidiMappings();

    // Set the current context (so mappings can adapt)
    void setContext(AppContext context);
    AppContext getContext() const { return currentContext; }

    // Process a MIDI event and return the corresponding action
    MidiAction processEvent(const MidiController::MidiEvent& midiEvent);

    // Convert MidiAction to ButtonEvent (for compatibility with existing screens)
    Controls::ButtonEvent actionToButtonEvent(const MidiAction& action,
                                               Controls* controls);

    // Configuration for Akai MPK Mini
    struct AkaiMPKMiniConfig {
        // Pad note numbers (configured for user's MPK Mini)
        uint8_t padNotes[8] = {37, 36, 42, 54, 40, 38, 46, 44};

        // Knob CC numbers (CC 1-8)
        uint8_t knobCCs[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    };

    AkaiMPKMiniConfig akaiConfig;

   private:
    AppContext currentContext;

    // Store last CC values to detect changes and direction
    uint8_t lastCCValues[128];

    // Track which side of selection we're editing (for knob 5/6)
    bool editingStartPosition;  // true = start, false = end

    // Context-specific mapping methods
    MidiAction mapHomeContext(const MidiController::MidiEvent& midiEvent);
    MidiAction mapRecorderContext(const MidiController::MidiEvent& midiEvent);
    MidiAction mapLiveContext(const MidiController::MidiEvent& midiEvent);

    // Helper to find pad index from note number
    int8_t getPadIndex(uint8_t note);

    // Helper to find knob index from CC number
    int8_t getKnobIndex(uint8_t ccNumber);
};

#endif
