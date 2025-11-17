#ifndef RecorderScreen_h
#define RecorderScreen_h

#include <Arduino.h>
#include <SD.h>

#include "../../hardware/Controls.h"
#include "../../helper/AudioResources.h"
#include "../../helper/NameGenerator.hpp"
#include "../../helper/WavFileWriter.hpp"
#include "../../main.h"
#include "../Screen.h"
#include "components/VolumeBar.h"
#include "components/waveform/Waveform.h"
#include "components/waveform/WaveformSelector.hpp"

class RecorderScreen {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);

    RecorderScreen(Controls* keyboard, Screen* screen,
                   NavigationCallback navCallback = nullptr);
    ~RecorderScreen();

    long receiveTimerTick();
    void handleEvent(Controls::ButtonEvent);
    void refresh();
    void setAudioResources(AudioResources* audioResources);

    void showRecorderScreen();
    void showEditScreen();
    void updateWaveform();

    void startRecording();
    void continueRecording();
    void stopRecording();
    void updateVolumeBar();

    enum RecorderState {
        RECORDER_HOME = 0,
        RECORDER_RECORDING = 1,
        RECORDER_EDITING = 2
    };

    static String getFilePath(const String& fileName) {
        return "/RECORDINGS/" + fileName + ".wav";
    }

    RecorderState currentState = RECORDER_HOME;

   private:
    NavigationCallback _navCallback;
    VolumeBar _volumeBar;
    Waveform _waveform;
    WaveformSelector _waveformSelector;
    Controls* _keyboard;
    Screen* _screen;
    int _selectedIndex = 0;

    AudioResources* _audioResources;
    WavFileWriter* _wavWriter;
    unsigned long _recordingStartTime = 0;
    String _recordedFileName;
    NameGenerator gen;

    // Rate limiting for display updates
    unsigned long _lastDisplayUpdate = 0;
    bool _needsDisplayUpdate = false;
    static const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 50;  // 20 Hz max

    void updateDisplayIfNeeded();
};

#endif
