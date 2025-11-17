# CLAUDE.md - Noisemaker Project Guide for AI Assistants

## Project Overview

**Noisemaker** is an embedded audio sampler/recorder built for the Teensy 4.0 microcontroller. It's a hardware-based audio device with a display interface and physical controls for recording, playing, and manipulating audio samples.

### Hardware Platform
- **Microcontroller**: Teensy 4.0 (ARM Cortex-M7 @ 600MHz)
- **Display**: SH1106 128x64 OLED (I2C on Wire2)
- **Audio**: SGTL5000 Audio Shield (I2S)
- **Controls**:
  - 1x Rotary Encoder (pins 2, 4)
  - 3x Buttons (pins 0, 1, 3)
- **Storage**: SD card for audio samples

### Technology Stack
- **Framework**: Arduino
- **Build System**: PlatformIO
- **Language**: C++ (Arduino dialect)

## Codebase Structure

```
noisemaker/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp           # Application entry point
│   ├── main.h             # App context enum definitions
│   ├── gui/               # Display and UI components
│   │   ├── Screen.cpp/h   # Display wrapper (U8g2)
│   │   └── screens/       # Individual screen implementations
│   │       ├── HomeScreen.cpp/h      # Main menu
│   │       └── RecorderScreen.cpp/h  # Recording interface
│   ├── hardware/          # Hardware abstraction
│   │   └── Controls.cpp/h # Button/encoder management
│   └── helper/            # Utility classes
│       ├── AudioResources.cpp/h  # Audio hardware setup
│       ├── FSIO.cpp/h           # File I/O utilities
│       └── SampleFSIO.cpp/h     # Sample-specific file ops
├── lib/                   # Project-specific libraries
├── include/               # Header files
└── test/                  # Test files
```

## Architecture Patterns

### 1. Context-Based Navigation
The application uses a state machine with three main contexts:
- **HOME**: Main menu and navigation
- **RECORDER**: Audio recording and editing
- **LIVE**: Live performance mode (planned)

```cpp
enum AppContext { HOME = 0, RECORDER = 1, LIVE = 2 };
```

Context switching is handled via `changeContext()` function in `src/main.cpp:38-55`.

### 2. Event-Driven Input Handling
Hardware events flow through a callback system:
```
Controls.tick() → Event Detection → handleControlEvent() → sendEventToActiveContext() → Screen.handleEvent()
```

Each screen implements its own `handleEvent()` method to process button and encoder events.

### 3. Screen Management
The `Screen` class (`src/gui/Screen.h`) wraps U8g2 display library and provides:
- Drawing primitives (text, boxes, lists)
- Display lifecycle management (begin, clear, display)
- Coordinate system utilities

Each screen type (HomeScreen, RecorderScreen) manages its own:
- UI state and rendering
- Event handling logic
- Navigation callbacks

## Key Components

### Controls (`src/hardware/Controls.h`)
Manages all physical input:
- **ButtonEvent structure**: Encodes button ID, state (NOT_PRESSED/PRESSED/LONG_PRESSED), encoder value
- **EventCallback**: Function pointer for event notifications
- **Static instance pattern**: Required for OneButton library callbacks

Pin assignments:
```cpp
buttonPin1 = 0
buttonPin2 = 1
buttonPin3 = 3
encoderPinA = 2
encoderPinB = 4
```

### AudioResources (`src/helper/AudioResources.h`)
Configures Teensy Audio Library components:
- I2S audio input/output
- SGTL5000 shield control
- Recording queue
- Peak analysis
- Volume/gain management

### RecorderScreen States
```cpp
RECORDER_HOME = 0      // Idle/menu
RECORDER_RECORDING = 1 // Active recording
RECORDER_EDITING = 2   // Post-recording edit
```

## Development Workflow

### Building and Uploading

**Build the project:**
```bash
pio run
```

**Upload to Teensy:**
```bash
pio run --target upload
```

**Clean build artifacts:**
```bash
pio run --target clean
```

**Open serial monitor:**
```bash
pio device monitor
```

### PlatformIO Configuration
See `platformio.ini` for build settings:
- Platform: `teensy`
- Board: `teensy40`
- Framework: `arduino`
- Libraries:
  - `olikraus/U8g2@^2.36.12` - Display driver
  - `mathertel/OneButton` - Button debouncing
  - `Encoder` - Rotary encoder handling

### Git Workflow
- Feature branches follow pattern: `claude/claude-md-<session-id>`
- Always develop on designated branch
- Commit messages should be descriptive
- Push with: `git push -u origin <branch-name>`

## Code Conventions

### File Organization
- **Headers (.h)**: Class declarations, includes guards required
- **Implementation (.cpp)**: Class definitions
- **Naming**: PascalCase for classes, camelCase for variables/functions
- **Include guards**: `#ifndef ClassName_h` format

### Memory Management
- **Avoid dynamic allocation**: Use static/stack allocation when possible
- **String handling**: Prefer `const char*` over String objects
- **PROGMEM**: Consider for large constant data (not currently used)

### Display Rendering
The U8g2 library uses a full framebuffer mode (`_F_` suffix):
1. `screen.clear()` - Clear buffer
2. Draw operations (drawStr, drawBox, etc.)
3. `screen.display()` - Send buffer to display

### Hardware Constraints
- **RAM**: 1MB DTCM (fast), 512KB OCRAM
- **Flash**: 2MB
- **Audio sample rate**: 44.1kHz typical
- **Display update**: Keep under 60Hz for smooth UI

## Common Tasks for AI Assistants

### Adding a New Screen
1. Create `ScreenName.h` and `ScreenName.cpp` in `src/gui/screens/`
2. Inherit pattern from HomeScreen/RecorderScreen
3. Implement `handleEvent()` and `refresh()` methods
4. Add navigation callback support
5. Update AppContext enum in `main.h`
6. Add context case in `changeContext()` and `sendEventToActiveContext()`

### Modifying UI Layout
- Screen area: 128x64 pixels (0,0 = top-left)
- Text uses U8g2 fonts (default: ~8px height)
- `drawItemList()` helper for menu-style lists
- Remember to call `screen.display()` after drawing

### Audio Processing Changes
- Teensy Audio Library uses patch cable connections
- Modify AudioResources class for new audio nodes
- Update setup in AudioResources constructor
- Maximum 200 audio blocks by default

### Button Mapping Changes
- Update pin definitions in `Controls.h`
- Modify OneButton initialization in `Controls.cpp`
- Update event routing if adding buttons

## Debugging

### Serial Debugging
```cpp
Serial.begin(9600);  // Already in setup()
Serial.println("Debug message");
```

### Common Issues
- **Display not working**: Check I2C connections on Wire2, verify address
- **No audio**: Verify Audio library memory allocation, check SD card
- **Button not responding**: Check pin conflicts, verify OneButton tick() called
- **Build errors**: Check library versions in platformio.ini

## Testing Considerations
- Test on actual hardware (no reliable emulator for Teensy)
- Serial output for state verification
- Audio library provides CPU usage stats: `AudioProcessorUsageMax()`
- Monitor memory: `AudioMemoryUsageMax()`

## External Dependencies

### Libraries (auto-installed via PlatformIO)
- **U8g2**: Monochrome graphics library
- **OneButton**: Debounced button handling with click/long-press
- **Encoder**: Quadrature encoder reading
- **Audio**: Teensy Audio Library (built-in)
- **SD/SPI/SerialFlash**: Storage access

### Documentation Links
- [PlatformIO Docs](https://docs.platformio.org/)
- [Teensy 4.0](https://www.pjrc.com/store/teensy40.html)
- [U8g2 Reference](https://github.com/olikraus/u8g2/wiki)
- [Teensy Audio Library](https://www.pjrc.com/teensy/td_libs_Audio.html)

## Future Architecture Notes

### Planned Features (from code comments)
- **LIVE context**: Performance mode implementation
- Complete audio playback integration
- Sample editing capabilities

### Extension Points
- Add more screens by extending Screen base pattern
- New audio effects via Audio library nodes
- Additional control schemes (MIDI, CV)
- Waveform visualization using display

## File I/O Patterns
- `FSIO`: General filesystem operations
- `SampleFSIO`: Audio sample-specific read/write
- WAV file format handling
- SD card reliability considerations (always properly close files)

## Important Notes for AI Assistants

1. **Always verify hardware compatibility** - This runs on embedded hardware, not a PC
2. **Memory is limited** - Avoid large allocations, monitor usage
3. **Timing matters** - Audio callbacks are real-time, minimize blocking operations
4. **Test incrementally** - Hardware bugs are harder to debug than software
5. **Comment hardware-specific code** - Future developers may not have EE background
6. **Preserve existing patterns** - Context/Screen/Controls architecture is intentional
7. **Check pin conflicts** - Teensy pins are shared between peripherals
8. **Build before committing** - Catch compilation errors early

## Questions or Issues?
When uncertain about:
- Hardware capabilities → Check Teensy 4.0 datasheet
- Audio library usage → Reference PJRC Audio Library documentation
- Display operations → Consult U8g2 wiki
- Build issues → Verify PlatformIO configuration

---

**Last Updated**: 2025-11-17
**Target Platform**: Teensy 4.0
**Framework**: Arduino/PlatformIO
