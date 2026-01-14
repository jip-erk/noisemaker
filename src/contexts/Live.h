#ifndef Live_h
#define Live_h

#include <Arduino.h>
#include <SD.h>

#include "../gui/Screen.h"
#include "../gui/screens/LiveScreen.h"
#include "../hardware/Controls.h"
#include "../helper/AudioResources.h"
#include "../helper/Track.hpp"
#include "../main.h"

class Live {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);

    Live(Controls* keyboard, Screen* screen,
         NavigationCallback navCallback = nullptr);
    ~Live();

    void refresh();
    void updateDisplay();
    void handleEvent(Controls::ButtonEvent);
    void setAudioResources(AudioResources* audioResources);
    long receiveTimerTick();

    enum LiveState {
        LIVE_MAIN = 0,           // Main sequencer view (all tracks in one page)
        LIVE_SAMPLE_SELECT = 1   // Selecting a sample for a track
    };

    LiveState currentState = LIVE_MAIN;

   private:
    NavigationCallback _navCallback;
    Controls* _keyboard;
    Screen* _screen;
    AudioResources* _audioResources;
    LiveScreen _liveScreen;

    // Track management
    static const int NUM_TRACKS = 4;
    static const int NUM_STEPS = 16;
    Track _tracks[NUM_TRACKS];
    int _selectedTrackIndex = 0;

    // Track labels for display
    static const char* TRACK_LABELS[NUM_TRACKS];

    // Sequencer data
    bool _sequencerGrid[NUM_TRACKS][NUM_STEPS];  // Step pattern data
    int _currentStep = 0;                         // Playhead position
    int _currentBPM = 120;                        // BPM (60-180)
    bool _isPlaying = false;                      // Playback state
    int _currentPage = 0;                         // Page (0-3 for step ranges 0-3, 4-7, 8-11, 12-15)

    // File list for sample selection
    int _selectedFileIndex = 0;
    int _fileCount = 0;
    String _fileList[20];  // Max 20 files

    // Display update flag (decoupled from audio timing)
    bool _displayNeedsUpdate = false;

    // Memory tracking
    unsigned long _lastMemoryLogTime = 0;
    static const unsigned long MEMORY_LOG_INTERVAL = 5000;  // Log every 5 seconds

    // Button combination tracking for order-dependent behaviors
    bool _button1WasPressed = false;
    bool _button2WasPressed = false;
    bool _button3WasPressed = false;
    bool _button4WasPressed = false;
    bool _button5UsedForCombo = false;  // Track if B5 was used in a combo

    // Private methods - business logic
    void loadFileList();
    void assignSampleToTrack();
    void clearTrack(int trackIndex);
    void playTrack(int trackIndex);
    void stopTrack(int trackIndex);
    void stopAllTracks();
    String getFileNameWithoutExtension(const String& fileName);
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
