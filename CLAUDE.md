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
│   ├── main.cpp           # Application entry point & audio setup
│   ├── main.h             # App context enum definitions
│   ├── gui/               # Display and UI components
│   │   ├── Screen.cpp/h   # Display wrapper (U8g2) with font management
│   │   └── screens/       # Individual screen implementations
│   │       ├── HomeScreen.cpp/h      # Main menu
│   │       ├── RecorderScreen.cpp/h  # Recording & waveform editing
│   │       ├── LiveScreen.cpp/h      # Sample playback interface
│   │       └── components/           # Reusable UI components
│   │           ├── TextHeader.cpp/h
│   │           ├── VolumeBar.cpp/h
│   │           └── waveform/
│   │               ├── Waveform.cpp/h          # Waveform visualization
│   │               └── WaveformSelector.hpp     # Waveform editing UI
│   ├── hardware/          # Hardware abstraction
│   │   └── Controls.cpp/h # Button/encoder with hold detection
│   └── helper/            # Utility classes
│       ├── AudioResources.cpp/h      # Audio routing & USB output
│       ├── FSIO.cpp/h               # File I/O utilities
│       ├── WavFileWriter.cpp/hpp    # WAV recording engine
│       ├── NameGenerator.hpp        # Random sample naming
│       └── audio-extensions/
│           └── play_sd_wav_extended.cpp/h  # Extended WAV playback
├── lib/                   # Project-specific libraries
├── include/               # Header files
└── test/                  # Test files
```

## Architecture Patterns

### 1. Context-Based Navigation
The application uses a state machine with three main contexts:
- **HOME**: Main menu and navigation
- **RECORDER**: Audio recording with waveform visualization and editing
- **LIVE**: Sample playback and performance mode

```cpp
enum AppContext { HOME = 0, RECORDER = 1, LIVE = 2 };
```

Context switching is handled via `changeContext()` function in `src/main.cpp:50-67`.
All contexts are fully implemented with dedicated screen classes.

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
- Font management (header vs normal fonts)
- Access to underlying U8g2 display via `getDisplay()`

Each screen type (HomeScreen, RecorderScreen, LiveScreen) manages its own:
- UI state and rendering
- Event handling logic
- Navigation callbacks
- Component composition (headers, volume bars, waveforms)

### 4. Component-Based UI
Reusable UI components (`src/gui/screens/components/`) enable modular interfaces:
- **TextHeader**: Centered text header display
- **VolumeBar**: Dual-channel audio level visualization
- **Waveform**: Real-time and cached waveform rendering with zoom/pan
- **WaveformSelector**: Interactive waveform region selection for editing

### 5. IntervalTimer System
A global interval timer (`src/main.cpp:31-149`) provides periodic updates:
- Configurable tick rate for each context
- RecorderScreen uses for waveform updates (70ms home, 500ms recording)
- Non-blocking update pattern to maintain UI responsiveness

```cpp
IntervalTimer globalTickTimer;
void globalTick();  // ISR sets ticked flag
void sendTickToActiveContext();  // Dispatches to active screen
```

## Key Components

### Controls (`src/hardware/Controls.h`)
Manages all physical input with advanced hold detection:
- **ButtonEvent structure**: Encodes button ID, state (NOT_PRESSED/PRESSED/LONG_PRESSED), encoder value, and button hold flags
- **Hold detection**: Tracks which buttons are currently held (`button1Held`, `button2Held`, `button3Held`)
- **Combo detection**: Enables button+encoder interactions (e.g., Button3+Encoder for zoom)
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

Button hold states enable complex interactions:
```cpp
// Example: Zoom with Button 3 + Encoder
if (event.button3Held && !event.button1Held && !event.button2Held) {
    _waveformSelector.zoom(event.encoderValue);
}
```

### AudioResources (`src/helper/AudioResources.h`)
Configures Teensy Audio Library components with USB audio output:
- **I2S audio input**: Microphone/line-in via SGTL5000 shield
- **USB audio output**: Streams to host computer (not I2S headphone out)
- **Audio mixers**: Complex routing with 3 mixer stages (recordMixer, recordInputMixer, mixer1)
- **Recording queue**: Buffered audio capture for WAV writing
- **Dual peak analysis**: Stereo input level monitoring (peak1, peak2)
- **Extended WAV playback**: AudioPlaySdWavExtended for region playback
- **Mute controls**: `muteInput()` and `unmuteInput()` for recording workflow

Build configuration (`platformio.ini`):
```cpp
-D USB_MIDI_AUDIO_SERIAL  // Enables USB audio + MIDI + serial
-D AUDIO_BLOCK_SAMPLES=128  // Reduced latency (default 256)
```

Audio graph connections (12 patch cords total) handle complex routing between:
- Input → Peak analysis
- Input → Recording mixers → Queue
- WAV playback → Output mixers
- All paths → USB output

### WavFileWriter (`src/helper/WavFileWriter.hpp`)
Handles real-time WAV file recording:
- **Streaming write**: Processes audio queue in chunks to avoid blocking
- **Accumulated buffer**: Provides waveform data for visualization (~0.5s buffer)
- **WAV header management**: Automatic header write and finalization
- **File operations**: `open()`, `update()`, `close()` lifecycle

Usage pattern:
```cpp
WavFileWriter writer(audioResources.queue1);
writer.open("/RECORDINGS/file.wav", 44100, 1);
while (recording) {
    writer.update();  // Call in main loop
    const int16_t* samples = writer.getAccumulatedBuffer(count);
    // Use samples for waveform visualization
}
writer.close();  // Finalizes WAV header
```

### NameGenerator (`src/helper/NameGenerator.hpp`)
Generates random memorable filenames for recordings:
- **PROGMEM storage**: 50 adjectives + 50 nouns stored in flash memory
- **2500 combinations**: "BrightWave", "DarkStorm", etc.
- **No collisions**: Random selection avoids manual naming
- **Memory efficient**: Uses `strcpy_P()` to copy from PROGMEM to RAM

### Screen States

**RecorderScreen States:**
```cpp
RECORDER_HOME = 0      // Idle/menu with volume monitoring
RECORDER_RECORDING = 1 // Active recording with live waveform
RECORDER_EDITING = 2   // Post-recording waveform editing
```

**LiveScreen States:**
```cpp
LIVE_HOME = 0      // File browser
LIVE_PLAYING = 1   // Playback in progress
LIVE_PAUSED = 2    // Playback paused
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
- Upload protocol: `teensy-cli`
- Build flags:
  - `USB_MIDI_AUDIO_SERIAL` - USB Audio/MIDI/Serial composite device
  - `AUDIO_BLOCK_SAMPLES=128` - Lower latency audio blocks (vs default 256)
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
- **String handling**: Prefer `const char*` over String objects, but String is used for file operations
- **PROGMEM**: Used for large constant data (see NameGenerator adjectives/nouns arrays)
- **Audio memory**: Allocated at startup via `AudioMemory(100)` - 100 blocks × 128 samples each
- **Monitor usage**: Check `AudioMemoryUsageMax()` and `AudioMemoryUsageMaxReset()`

### Display Rendering
The U8g2 library uses a full framebuffer mode (`_F_` suffix):
1. `screen.clear()` - Clear buffer
2. Draw operations (drawStr, drawBox, etc.)
3. `screen.display()` - Send buffer to display

Font management:
- **Header font**: `u8g2_font_doomalpha04_tr` via `screen.setHeaderFont()`
- **Normal font**: `u8g2_font_tiny5_tr` via `screen.setNormalFont()`
- Font changes persist until changed again
- Always reset to normal font after using header font

Component rendering pattern:
```cpp
screen.clear();
screen.setHeaderFont();
screen.drawStr(0, 10, "RECORDER");
screen.setNormalFont();
volumeBar.drawVolumeBar();  // Component draws itself
waveform.drawWaveform();    // Component draws itself
screen.display();
```

### Hardware Constraints
- **RAM**: 1MB DTCM (fast), 512KB OCRAM
- **Flash**: 2MB
- **Audio sample rate**: 44.1kHz typical
- **Display update**: Keep under 60Hz for smooth UI

## Common Tasks for AI Assistants

### Adding a New Screen
1. Create `ScreenName.h` and `ScreenName.cpp` in `src/gui/screens/`
2. Inherit pattern from HomeScreen/RecorderScreen/LiveScreen
3. Implement `handleEvent()` and `refresh()` methods
4. Add navigation callback support
5. Add `setAudioResources()` if audio access needed
6. Add `receiveTimerTick()` if periodic updates needed
7. Update AppContext enum in `main.h` if adding new context
8. Add context case in `changeContext()`, `sendEventToActiveContext()`, and `sendTickToActiveContext()`
9. Instantiate screen in `main.cpp` and pass dependencies

### Creating UI Components
1. Create class in `src/gui/screens/components/`
2. Constructor takes `Screen*` pointer and position/size parameters
3. Provide `draw()` or similar method for rendering
4. Component manages its own state
5. Example pattern from VolumeBar:
```cpp
VolumeBar(Screen* screen, int x, int y, int width, int height);
void setLeftVolume(float left);
void setRightVolume(float right);
void drawVolumeBar();
```

### Modifying UI Layout
- Screen area: 128x64 pixels (0,0 = top-left)
- Header font: ~8px height, normal font: ~5px height
- Use `screen.getWidth()` for dynamic centering
- `drawItemList()` helper for menu-style lists
- Components can use `screen.getDisplay()` for advanced U8g2 features
- Remember to call `screen.display()` after drawing
- Inverted text: `setDrawColor(0)` with white background box

### Working with Waveforms
- **Waveform class**: Handles rendering of audio data
- **Real-time updates**: `addAudioData()` during recording
- **Cached loading**: `loadWaveformFile()` for editing existing files
- **Zoom/pan**: WaveformSelector provides interactive editing
- **Memory management**: Waveform downsamples to fit screen width
- WAV files store samples with 44-byte header offset

### Audio Processing Changes
- Teensy Audio Library uses patch cable connections
- Modify AudioResources class for new audio nodes
- Update setup in AudioResources constructor
- Audio memory allocated as blocks: `AudioMemory(100)` = 100 × 128 samples
- USB audio output requires USB_MIDI_AUDIO_SERIAL build flag
- Input muting: Use `audioResources.muteInput()` / `unmuteInput()`

### Button Mapping Changes
- Update pin definitions in `Controls.h`
- Modify OneButton initialization in `Controls.cpp`
- Update event routing if adding buttons
- Consider button hold state tracking for combos
- Button hold flags: `button1Held`, `button2Held`, `button3Held`

### SD Card File Operations
- All recordings go to `/RECORDINGS/` directory
- Create directory if not exists: `SD.mkdir("/RECORDINGS")`
- WAV file naming via NameGenerator for uniqueness
- Always close files properly: `file.close()`
- Check existence: `SD.exists(path)`
- Directory scanning pattern: `openNextFile()` loop (see LiveScreen)
- File path helper: `RecorderScreen::getFilePath(name)`

## Debugging

### Serial Debugging
```cpp
Serial.begin(9600);  // Already in setup()
Serial.println("Debug message");
```

### Common Issues
- **Display not working**: Check I2C connections on Wire2, verify SH1106 address, ensure `screen.begin()` called
- **No audio input**: Verify Audio library memory allocation (`AudioMemory(100)`), check SGTL5000 shield initialization, ensure input not muted
- **No audio output**: USB audio requires USB_MIDI_AUDIO_SERIAL flag, check host audio settings
- **Button not responding**: Check pin conflicts, verify `OneButton tick()` called in main loop
- **Build errors**: Check library versions in platformio.ini, verify build flags
- **SD card errors**: Check SPI pin configuration (CS=10, MOSI=11, SCK=13), verify SD card formatting (FAT32)
- **Waveform not displaying**: Check WAV file header (44 bytes), verify file path, ensure sufficient memory
- **Recording creates empty files**: Verify audio input levels, check `continueRecording()` called in loop, ensure queue is being read

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

### Implemented Features
- ✅ **LIVE context**: Sample browser and playback (LiveScreen)
- ✅ **Audio playback**: Extended WAV playback with region selection
- ✅ **Sample editing**: Waveform visualization and region selection for playback
- ✅ **Recording workflow**: Real-time waveform display and automatic file naming
- ✅ **USB audio output**: Direct streaming to host computer

### Potential Enhancements
- Sample trimming and saving edited regions
- Multiple sample playback (polyphony)
- Effects processing (filters, reverb, delay)
- MIDI note triggering for samples
- Metronome/click track
- Loop point editing
- Sample pitch shifting

### Extension Points
- Add more screens by extending Screen base pattern
- New audio effects via Audio library nodes
- Additional control schemes (MIDI, CV)
- Waveform visualization using display

## File I/O Patterns

### FSIO (`src/helper/FSIO.h`)
General filesystem operations (basic file utilities)

### WavFileWriter (`src/helper/WavFileWriter.hpp`)
Streaming WAV recording with real-time header management:
- Opens file and writes 44-byte WAV header
- Continuously reads from AudioRecordQueue
- Accumulates samples for waveform visualization
- Finalizes header with correct byte count on close

### AudioPlaySdWavExtended (`src/helper/audio-extensions/`)
Enhanced WAV playback with region selection:
- Supports byte-range playback for edited regions
- Maintains compatibility with standard AudioPlaySdWav
- Used by LiveScreen for sample playback

### Recording Workflow
1. User initiates recording in RecorderScreen
2. WavFileWriter creates `/RECORDINGS/{RandomName}.wav`
3. `continueRecording()` called in main loop → `writer.update()`
4. WavFileWriter provides accumulated samples to Waveform component
5. Waveform downsamples and displays real-time visualization
6. On stop, `writer.close()` finalizes WAV header
7. Transition to editing mode with cached waveform

### SD Card Best Practices
- Always close files: `file.close()`
- Check return values: `if (!file) { /* error */ }`
- Minimize write frequency to extend card life
- Use buffered writes (512 bytes) for efficiency
- Verify SD card before operations: `SD.begin(SDCARD_CS_PIN)`
- Handle missing directories gracefully

## Important Notes for AI Assistants

1. **Always verify hardware compatibility** - This runs on embedded hardware, not a PC
2. **Memory is limited** - Avoid large allocations, monitor usage with `AudioMemoryUsageMax()`
3. **Timing matters** - Audio callbacks are real-time, minimize blocking operations
4. **Test incrementally** - Hardware bugs are harder to debug than software
5. **Comment hardware-specific code** - Future developers may not have EE background
6. **Preserve existing patterns** - Context/Screen/Controls architecture is intentional
7. **Check pin conflicts** - Teensy pins are shared between peripherals
8. **Build before committing** - Catch compilation errors early
9. **USB audio implications** - Volume control happens on host, not device
10. **Component lifecycle** - Components should be instantiated as members, not dynamically allocated
11. **Main loop must stay responsive** - Long operations should be broken into chunks
12. **IntervalTimer runs in ISR** - Keep `globalTick()` minimal, just set flag
13. **String objects are OK for file paths** - But avoid in real-time audio code
14. **PROGMEM for constants** - Use for lookup tables and large arrays
15. **Button combos** - Check hold states before processing encoder events

## Feature Implementation Patterns

### Recording with Waveform Visualization
RecorderScreen demonstrates real-time audio visualization:
- WavFileWriter accumulates samples in buffer (~0.5 sec)
- `receiveTimerTick()` called periodically (500ms during recording)
- `updateWaveform()` pulls accumulated buffer and adds to Waveform
- Waveform downsamples to screen width (128 pixels)
- Component handles its own rendering

### Sample Playback with File Browser
LiveScreen shows SD card file browsing pattern:
- `loadFileList()` scans `/RECORDINGS/` directory
- Stores up to 20 filenames in array
- Encoder scrolls through list with highlighted selection
- AudioPlaySdWavExtended handles playback
- USB audio output streams to host

### Button Hold Detection
Controls class tracks button states:
- Events include hold flags (`button1Held`, `button2Held`, `button3Held`)
- Enables modifier key pattern (Button + Encoder)
- Check hold states in event handler to disambiguate actions
- Example: Button 3 + Encoder = Zoom, Encoder alone = Selection

### Component-Based UI
Screens compose reusable UI components:
- Pass Screen pointer to component constructor
- Components handle their own state and rendering
- Screen provides coordinate space and display access
- Keeps screen code clean and testable

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
**Version**: Post-merge with main (includes LiveScreen, WavFileWriter, Waveform editing)
