#ifndef Live_h
#define Live_h

#include <Arduino.h>
#include <SD.h>

#include "../gui/Screen.h"
#include "../gui/screens/LiveScreen.h"
#include "../hardware/Controls.h"
#include "../helper/AudioResources.h"
#include "../helper/StepSequencer.h"
#include "../helper/Track.hpp"
#include "../main.h"

class Live {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);
    typedef void (*TimerResetCallback)(long intervalMicros);

    Live(Controls* keyboard, Screen* screen,
         NavigationCallback navCallback = nullptr,
         TimerResetCallback timerCallback = nullptr);
    ~Live();

    void refresh();
    void updateDisplay();
    void handleEvent(Controls::ButtonEvent);
    void setAudioResources(AudioResources* audioResources);
    long receiveTimerTick();

    enum LiveState {
        LIVE_MAIN = 0,          // Main sequencer view (all tracks in one page)
        LIVE_SAMPLE_SELECT = 1  // Selecting a sample for a track
    };

    LiveState currentState = LIVE_MAIN;

   private:
    NavigationCallback _navCallback;
    TimerResetCallback _timerCallback;
    Controls* _keyboard;
    Screen* _screen;
    AudioResources* _audioResources;
    LiveScreen _liveScreen;

    // Track management
    static const int NUM_TRACKS = 7;
    static const int NUM_STEPS = 32;
    Track _tracks[NUM_TRACKS];
    int _selectedTrackIndex = 0;

    // Track volume control (0.0 to 1.0)
    float _trackVolumes[NUM_TRACKS];
    float _trackPitch[NUM_TRACKS];  // 0.5 = normal, 0..2 range

    // Track labels for display
    static const char* TRACK_LABELS[NUM_TRACKS];

    // Volume control constants
    static constexpr float VOLUME_STEP = 0.05f;  // 5% per encoder tick
    static constexpr float VOLUME_MIN = 0.0f;
    static constexpr float VOLUME_MAX = 4.0f;

    // Pitch control constants
    static constexpr float PITCH_STEP = 0.05f;
    static constexpr float PITCH_MIN = 0.1f;
    static constexpr float PITCH_MAX = 2.0f;

    // Sequencer
    StepSequencer _sequencer;
    int _currentPage = 0;  // Page (0-3 for step ranges 0-3, 4-7, 8-11, 12-15)
    int _stepRange = 0;    // Range offset (0 or 16)

    // File list for sample selection
    int _selectedFileIndex = 0;
    int _fileCount = 0;
    static const int MAX_FILES = 50;
    String _fileList[MAX_FILES];  // Max files

    // Display update flag (decoupled from audio timing)
    bool _displayNeedsUpdate = false;

    // Memory tracking
    unsigned long _lastMemoryLogTime = 0;
    static const unsigned long MEMORY_LOG_INTERVAL =
        5000;  // Log every 5 seconds

    // Button combination tracking for order-dependent behaviors
    bool _button1WasPressed = false;
    bool _button2WasPressed = false;
    bool _button3WasPressed = false;
    bool _button4WasPressed = false;
    bool _button5UsedForCombo = false;  // Track if B5 was used in a combo

    // Control state
    bool _controlActive = false;
    bool _controlWasUsed =
        false;  // Track if control was used to prevent step toggle

    // Private methods - business logic
    void loadFileList();
    void assignSampleToTrack();
    void clearTrack(int trackIndex);
    void playTrack(int trackIndex);
    void stopTrack(int trackIndex);
    void stopAllTracks();
    String getFileNameWithoutExtension(const String& fileName);
    void setTrackVolume(int trackIndex, float volume);
    float getTrackVolume(int trackIndex) const;
    void setTrackPitch(int trackIndex, float pitch);
    float getTrackPitch(int trackIndex) const;
    void updateTrackDisplay();

    // Sequencer methods
    void startPlayback();
    void stopPlayback();
    void toggleStep(int track, int step);
    void advanceStep();
    int calculateStepIntervalMicros();
    void updateLEDs();
    void logMemoryUsage();
    void cacheSamples();
};

#endif
