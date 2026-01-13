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
    void handleEvent(Controls::ButtonEvent);
    void setAudioResources(AudioResources* audioResources);
    long receiveTimerTick();

    enum LiveState {
        LIVE_TRACK_VIEW = 0,     // Viewing/selecting tracks
        LIVE_SAMPLE_SELECT = 1,  // Selecting a sample for a track
        LIVE_SEQUENCER = 2       // Sequencer grid view
    };

    LiveState currentState = LIVE_TRACK_VIEW;

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

    // File list for sample selection
    int _selectedFileIndex = 0;
    int _fileCount = 0;
    String _fileList[20];  // Max 20 files

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
};

#endif
