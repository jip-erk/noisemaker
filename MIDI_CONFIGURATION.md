# MIDI Configuration Guide for Noisemaker

## Overview

The Noisemaker now supports MIDI input for controlling parameters and navigation! The MIDI system is fully modular and context-aware, meaning the same MIDI controller behaves differently depending on which screen you're on (Home, Recorder, or Live).

## Architecture

### Components

1. **MidiController** (`src/hardware/MidiController.h/cpp`)
   - Handles raw MIDI I/O via USB MIDI
   - Supports Note On/Off, Control Change, Program Change, Pitch Bend
   - Polls MIDI input and triggers callbacks

2. **MidiMappings** (`src/hardware/MidiMappings.h/cpp`)
   - Context-aware mapping system
   - Translates MIDI messages to actions based on current screen
   - Converts MIDI actions to ButtonEvents for compatibility

3. **Integration** (in `src/main.cpp`)
   - MIDI controller ticks in main loop alongside physical controls
   - Events routed through the same system as buttons/encoder
   - Existing screens work without modification

## Default Configuration: Akai MPK Mini

The system comes pre-configured for the **Akai MPK Mini** with smart defaults:

### Pad Mappings

**Default note numbers**: 36-43 (Pads 1-8)

#### HOME Context
- **Pad 1** (Note 36): Navigate Up
- **Pad 2** (Note 37): Navigate Down
- **Pad 3** (Note 38): Select/Enter
- **Pad 4** (Note 39): Back/Cancel
- **Pads 5-8**: Available for future features

#### RECORDER Context
- **Pad 1** (Note 36): Record/Stop
- **Pad 2** (Note 37): Navigate Up
- **Pad 3** (Note 38): Navigate Down
- **Pad 4** (Note 39): Select/Enter
- **Pad 5** (Note 40): Back/Cancel
- **Pad 6** (Note 41): Button 2 (for combos)
- **Pads 7-8**: Available for future features

#### LIVE Context
- **Pad 1** (Note 36): Navigate Up
- **Pad 2** (Note 37): Navigate Down
- **Pad 3** (Note 38): Select/Enter
- **Pad 4** (Note 39): Back/Cancel
- **Pads 5-8**: Reserved for sample triggering (future)

### Knob Mappings

**Default CC numbers**: 70-77 (Knobs 1-8)

- **Knob 1** (CC 70): Main navigation encoder simulation (works in all contexts)
- **Knobs 2-8**: Available for context-specific parameters

## How to Customize

### Changing Pad/Knob Numbers

Edit `src/hardware/MidiMappings.h` and modify the `AkaiMPKMiniConfig` structure:

```cpp
struct AkaiMPKMiniConfig {
    // Change these to match your controller
    uint8_t padNotes[8] = {36, 37, 38, 39, 40, 41, 42, 43};
    uint8_t knobCCs[8] = {70, 71, 72, 73, 74, 75, 76, 77};
};
```

### Adding Context-Specific Mappings

Edit the mapping functions in `src/hardware/MidiMappings.cpp`:

- `mapHomeContext()` - For HOME screen behavior
- `mapRecorderContext()` - For RECORDER screen behavior
- `mapLiveContext()` - For LIVE screen behavior

#### Example: Adding a Custom Action

```cpp
MidiAction MidiMappings::mapRecorderContext(
    const MidiController::MidiEvent& midiEvent) {

    int8_t padIndex = getPadIndex(midiEvent.getNote());

    if (midiEvent.isNoteOn() && padIndex == 6) {
        // Pad 7 does something custom
        return MidiAction(MIDI_ACTION_CUSTOM, 1, 1);
    }

    // ... rest of mapping logic
}
```

### Adding New Action Types

1. Add to the `MidiActionType` enum in `src/hardware/MidiMappings.h`:

```cpp
enum MidiActionType {
    // ... existing types
    MIDI_ACTION_CUSTOM_FEATURE = 101,
};
```

2. Handle in `actionToButtonEvent()` in `src/hardware/MidiMappings.cpp`

## MIDI Action Types

Available action types for mapping:

- **MIDI_ACTION_NONE**: No action
- **MIDI_ACTION_ENCODER**: Simulate encoder rotation (value = direction)
- **MIDI_ACTION_BUTTON**: Simulate button press (targetId = button number)
- **MIDI_ACTION_NAVIGATION_UP**: Navigate menu up
- **MIDI_ACTION_NAVIGATION_DOWN**: Navigate menu down
- **MIDI_ACTION_NAVIGATION_SELECT**: Select/confirm
- **MIDI_ACTION_NAVIGATION_BACK**: Go back/cancel
- **MIDI_ACTION_TRANSPORT_RECORD**: Record transport control
- **MIDI_ACTION_TRANSPORT_PLAY**: Play transport control
- **MIDI_ACTION_TRANSPORT_STOP**: Stop transport control
- **MIDI_ACTION_CUSTOM**: Custom actions (100+)

## Using a Different MIDI Controller

To use a controller other than Akai MPK Mini:

1. Connect your controller via USB to the Teensy 4.0
2. Determine the note numbers and CC numbers your controller sends:
   - Use a MIDI monitor tool on your computer
   - Or add Serial.println() in `MidiController::processMidiMessage()`
3. Update the configuration in `MidiMappings.h`
4. Customize mappings in `MidiMappings.cpp`

## Troubleshooting

### No MIDI Response

1. **Check USB MIDI is enabled**: Verify `platformio.ini` has `-D USB_MIDI_AUDIO_SERIAL`
2. **Check connection**: Ensure MIDI controller is connected to Teensy USB port
3. **Test MIDI input**: Add debug output in `handleMidiEvent()` in `main.cpp`:
   ```cpp
   void handleMidiEvent(MidiController::MidiEvent midiEvent) {
       Serial.print("MIDI: ");
       Serial.print(midiEvent.type);
       Serial.print(" ");
       Serial.println(midiEvent.data1);
       // ... rest of function
   }
   ```

### Wrong Notes/CCs Triggering Actions

- Your controller may send different note numbers than the defaults
- Use Serial monitoring to discover the actual values
- Update `AkaiMPKMiniConfig` accordingly

### Knobs Not Working Smoothly

- CC encoder simulation uses delta detection
- First CC value after context change is ignored (initialization)
- Try adjusting sensitivity in `mapXXXContext()` functions

## Technical Details

### Event Flow

```
MIDI Input → MidiController.tick()
           → handleMidiEvent()
           → MidiMappings.processEvent()
           → MidiAction
           → actionToButtonEvent()
           → ButtonEvent
           → sendEventToActiveContext()
           → Screen.handleEvent()
```

### Context Switching

When changing contexts (e.g., HOME → RECORDER):
1. `changeContext()` is called in `main.cpp`
2. `midiMappings.setContext()` updates the active mapping
3. CC value tracking is reset to prevent false triggers
4. New context's screen is refreshed

### Compatibility

- Existing screens require **no modification**
- MIDI events are translated to ButtonEvents
- Physical controls and MIDI can be used simultaneously
- No conflicts between control sources

## Performance Considerations

- **CPU Impact**: Minimal - MIDI polling is lightweight
- **Memory**: ~300 bytes for CC/note state tracking
- **Latency**: Sub-millisecond response time
- **Polling Rate**: Checked every loop iteration (~few kHz)

## Future Enhancements

Possible extensions to the MIDI system:

1. **MIDI Learn Mode**: Automatically detect and map controller
2. **Velocity Sensitivity**: Use pad velocity for dynamics
3. **MIDI Output**: Send MIDI from Teensy
4. **Persistent Configuration**: Store mappings in EEPROM
5. **Multi-Controller Support**: Handle multiple MIDI devices
6. **Advanced Mappings**: Ranges, curves, button combinations

## Contributing

To add new MIDI features:

1. Keep the modular architecture
2. Maintain backward compatibility with existing screens
3. Document new action types
4. Consider context-specific behavior
5. Test with physical hardware

## Examples

### Example 1: Using Knob 2 to Control Recording Level

In `src/hardware/MidiMappings.cpp`, modify `mapRecorderContext()`:

```cpp
else if (midiEvent.isCC() && knobIndex >= 0) {
    uint8_t currentValue = midiEvent.getCCValue();

    if (knobIndex == 1) {  // Knob 2
        // Map CC value (0-127) to recording level
        // This could be sent as a custom action
        return MidiAction(MIDI_ACTION_CUSTOM, 10, currentValue);
    }
}
```

### Example 2: Pad Velocity for Parameter Control

```cpp
if (midiEvent.isNoteOn()) {
    uint8_t velocity = midiEvent.getVelocity();
    // Use velocity to scale parameter
    return MidiAction(MIDI_ACTION_CUSTOM, padIndex, velocity);
}
```

## Summary

The MIDI system is:
- ✅ **Modular**: Easy to extend and customize
- ✅ **Context-Aware**: Behavior adapts to current screen
- ✅ **Non-Invasive**: Existing code unchanged
- ✅ **Flexible**: Supports any USB MIDI controller
- ✅ **Well-Integrated**: Uses existing event system

Enjoy controlling your Noisemaker with MIDI! 🎹🎵
