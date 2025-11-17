#include "MidiMappings.h"

MidiMappings::MidiMappings() : currentContext(AppContext::HOME) {
    // Initialize last CC values
    for (int i = 0; i < 128; i++) {
        lastCCValues[i] = 0;
    }
}

void MidiMappings::setContext(AppContext context) {
    currentContext = context;

    // Reset CC tracking when context changes to avoid false triggers
    for (int i = 0; i < 128; i++) {
        lastCCValues[i] = 0;
    }
}

MidiAction MidiMappings::processEvent(
    const MidiController::MidiEvent& midiEvent) {
    // Route to context-specific handler
    switch (currentContext) {
        case AppContext::HOME:
            return mapHomeContext(midiEvent);
        case AppContext::RECORDER:
            return mapRecorderContext(midiEvent);
        case AppContext::LIVE:
            return mapLiveContext(midiEvent);
        default:
            return MidiAction(MIDI_ACTION_NONE);
    }
}

Controls::ButtonEvent MidiMappings::actionToButtonEvent(
    const MidiAction& action, Controls* controls) {
    Controls::ButtonEvent event;
    event.encoderValue = 0;
    event.button1Held = controls->isDown(1);
    event.button2Held = controls->isDown(2);
    event.button3Held = controls->isDown(3);

    switch (action.actionType) {
        case MIDI_ACTION_ENCODER:
            // Simulate encoder rotation
            event.buttonId = 0;  // Encoder is button 0
            event.state = NOT_PRESSED;
            event.encoderValue = action.value;  // Direction: -1 or +1
            break;

        case MIDI_ACTION_BUTTON:
            // Simulate button press/release
            event.buttonId = action.targetId;
            event.state = (action.value > 0) ? PRESSED : NOT_PRESSED;
            break;

        case MIDI_ACTION_NAVIGATION_UP:
            // Navigate up = encoder backward
            event.buttonId = 0;
            event.state = NOT_PRESSED;
            event.encoderValue = -1;
            break;

        case MIDI_ACTION_NAVIGATION_DOWN:
            // Navigate down = encoder forward
            event.buttonId = 0;
            event.state = NOT_PRESSED;
            event.encoderValue = 1;
            break;

        case MIDI_ACTION_NAVIGATION_SELECT:
            // Select = button 1 press
            event.buttonId = 1;
            event.state = (action.value > 0) ? PRESSED : NOT_PRESSED;
            break;

        case MIDI_ACTION_NAVIGATION_BACK:
            // Back = button 3 press
            event.buttonId = 3;
            event.state = (action.value > 0) ? PRESSED : NOT_PRESSED;
            break;

        default:
            // For other actions, create a neutral event
            event.buttonId = 0;
            event.state = NOT_PRESSED;
            event.encoderValue = 0;
            break;
    }

    return event;
}

// HOME CONTEXT MAPPINGS
// Pads: Navigation and context switching
// Knobs: Not used in home (could add menu scrolling)
MidiAction MidiMappings::mapHomeContext(
    const MidiController::MidiEvent& midiEvent) {
    int8_t padIndex = getPadIndex(midiEvent.getNote());
    int8_t knobIndex = getKnobIndex(midiEvent.getCCNumber());

    if (midiEvent.isNoteOn()) {
        // Pad pressed
        if (padIndex >= 0) {
            switch (padIndex) {
                case 0:  // Pad 1: Navigate up
                    return MidiAction(MIDI_ACTION_NAVIGATION_UP, 0, 1);
                case 1:  // Pad 2: Navigate down
                    return MidiAction(MIDI_ACTION_NAVIGATION_DOWN, 0, 1);
                case 2:  // Pad 3: Select/Enter
                    return MidiAction(MIDI_ACTION_NAVIGATION_SELECT, 0, 1);
                case 3:  // Pad 4: Back
                    return MidiAction(MIDI_ACTION_NAVIGATION_BACK, 0, 1);
                // Pads 5-8 available for future use
                default:
                    break;
            }
        }
    } else if (midiEvent.isNoteOff()) {
        // Pad released - send button release
        if (padIndex >= 0) {
            switch (padIndex) {
                case 2:  // Pad 3: Select/Enter release
                    return MidiAction(MIDI_ACTION_NAVIGATION_SELECT, 0, 0);
                case 3:  // Pad 4: Back release
                    return MidiAction(MIDI_ACTION_NAVIGATION_BACK, 0, 0);
                default:
                    break;
            }
        }
    } else if (midiEvent.isCC() && knobIndex >= 0) {
        // Knobs can be used for scrolling in lists
        uint8_t currentValue = midiEvent.getCCValue();
        uint8_t lastValue = lastCCValues[midiEvent.getCCNumber()];
        lastCCValues[midiEvent.getCCNumber()] = currentValue;

        if (lastValue != 0) {  // Ignore first value (initialization)
            int8_t delta = currentValue - lastValue;
            if (delta != 0) {
                // Knob 1 controls menu navigation
                if (knobIndex == 0) {
                    return MidiAction(MIDI_ACTION_ENCODER, 0,
                                      (delta > 0) ? 1 : -1);
                }
            }
        }
    }

    return MidiAction(MIDI_ACTION_NONE);
}

// RECORDER CONTEXT MAPPINGS
// Pads: Transport controls + navigation
// Knobs: Recording parameters (levels, monitoring, etc)
MidiAction MidiMappings::mapRecorderContext(
    const MidiController::MidiEvent& midiEvent) {
    int8_t padIndex = getPadIndex(midiEvent.getNote());
    int8_t knobIndex = getKnobIndex(midiEvent.getCCNumber());

    if (midiEvent.isNoteOn()) {
        // Pad pressed
        if (padIndex >= 0) {
            switch (padIndex) {
                case 0:  // Pad 1: Record/Stop
                    return MidiAction(MIDI_ACTION_TRANSPORT_RECORD, 0, 1);
                case 1:  // Pad 2: Navigate up
                    return MidiAction(MIDI_ACTION_NAVIGATION_UP, 0, 1);
                case 2:  // Pad 3: Navigate down
                    return MidiAction(MIDI_ACTION_NAVIGATION_DOWN, 0, 1);
                case 3:  // Pad 4: Select/Enter
                    return MidiAction(MIDI_ACTION_NAVIGATION_SELECT, 0, 1);
                case 4:  // Pad 5: Back
                    return MidiAction(MIDI_ACTION_NAVIGATION_BACK, 0, 1);
                case 5:  // Pad 6: Button 2 (for combos)
                    return MidiAction(MIDI_ACTION_BUTTON, 2, 1);
                // Pads 7-8 available
                default:
                    break;
            }
        }
    } else if (midiEvent.isNoteOff()) {
        // Pad released
        if (padIndex >= 0) {
            switch (padIndex) {
                case 0:  // Record button release
                    return MidiAction(MIDI_ACTION_TRANSPORT_RECORD, 0, 0);
                case 3:  // Select release
                    return MidiAction(MIDI_ACTION_NAVIGATION_SELECT, 0, 0);
                case 4:  // Back release
                    return MidiAction(MIDI_ACTION_NAVIGATION_BACK, 0, 0);
                case 5:  // Button 2 release
                    return MidiAction(MIDI_ACTION_BUTTON, 2, 0);
                default:
                    break;
            }
        }
    } else if (midiEvent.isCC() && knobIndex >= 0) {
        // Knobs control recording parameters
        uint8_t currentValue = midiEvent.getCCValue();
        uint8_t lastValue = lastCCValues[midiEvent.getCCNumber()];
        lastCCValues[midiEvent.getCCNumber()] = currentValue;

        if (lastValue != 0) {  // Ignore first value
            int8_t delta = currentValue - lastValue;
            if (delta != 0) {
                // Knob 1: Main navigation encoder
                if (knobIndex == 0) {
                    return MidiAction(MIDI_ACTION_ENCODER, 0,
                                      (delta > 0) ? 1 : -1);
                }
                // Knobs 2-8 can control recording parameters
                // These can be expanded based on recorder needs
            }
        }
    }

    return MidiAction(MIDI_ACTION_NONE);
}

// LIVE CONTEXT MAPPINGS
// Pads: Trigger samples, performance controls
// Knobs: Live parameters (volume, effects, etc)
MidiAction MidiMappings::mapLiveContext(
    const MidiController::MidiEvent& midiEvent) {
    int8_t padIndex = getPadIndex(midiEvent.getNote());
    int8_t knobIndex = getKnobIndex(midiEvent.getCCNumber());

    if (midiEvent.isNoteOn()) {
        if (padIndex >= 0) {
            // In live mode, pads could trigger samples or perform actions
            // For now, basic navigation
            switch (padIndex) {
                case 0:  // Pad 1: Navigate up
                    return MidiAction(MIDI_ACTION_NAVIGATION_UP, 0, 1);
                case 1:  // Pad 2: Navigate down
                    return MidiAction(MIDI_ACTION_NAVIGATION_DOWN, 0, 1);
                case 2:  // Pad 3: Select
                    return MidiAction(MIDI_ACTION_NAVIGATION_SELECT, 0, 1);
                case 3:  // Pad 4: Back
                    return MidiAction(MIDI_ACTION_NAVIGATION_BACK, 0, 1);
                // Pads 5-8 for sample triggering (future)
                default:
                    break;
            }
        }
    } else if (midiEvent.isNoteOff()) {
        if (padIndex >= 0) {
            switch (padIndex) {
                case 2:
                    return MidiAction(MIDI_ACTION_NAVIGATION_SELECT, 0, 0);
                case 3:
                    return MidiAction(MIDI_ACTION_NAVIGATION_BACK, 0, 0);
                default:
                    break;
            }
        }
    } else if (midiEvent.isCC() && knobIndex >= 0) {
        uint8_t currentValue = midiEvent.getCCValue();
        uint8_t lastValue = lastCCValues[midiEvent.getCCNumber()];
        lastCCValues[midiEvent.getCCNumber()] = currentValue;

        if (lastValue != 0) {
            int8_t delta = currentValue - lastValue;
            if (delta != 0 && knobIndex == 0) {
                return MidiAction(MIDI_ACTION_ENCODER, 0,
                                  (delta > 0) ? 1 : -1);
            }
        }
    }

    return MidiAction(MIDI_ACTION_NONE);
}

int8_t MidiMappings::getPadIndex(uint8_t note) {
    for (int i = 0; i < 8; i++) {
        if (akaiConfig.padNotes[i] == note) {
            return i;
        }
    }
    return -1;  // Not a mapped pad
}

int8_t MidiMappings::getKnobIndex(uint8_t ccNumber) {
    for (int i = 0; i < 8; i++) {
        if (akaiConfig.knobCCs[i] == ccNumber) {
            return i;
        }
    }
    return -1;  // Not a mapped knob
}
