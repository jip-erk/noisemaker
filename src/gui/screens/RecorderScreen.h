#ifndef RecorderScreen_h
#define RecorderScreen_h

#include <Arduino.h>

#include "../../hardware/Controls.h"
#include "../../helper/AudioResources.h"
#include "../../helper/FSIO.h"
#include "../../helper/WavFileWriter.hpp"
#include "../../main.h"
#include "../Screen.h"

class RecorderScreen {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);

    RecorderScreen(Controls *keyboard, Screen *screen,
                   NavigationCallback navCallback = nullptr);
    ~RecorderScreen();

    void handleEvent(Controls::ButtonEvent);
    void refresh();
    void setAudioResources(AudioResources* audioResources);

    void showRecorderScreen();

    void startRecording();
    void continueRecording();
    void stopRecording();

    enum RecorderState {
        RECORDER_HOME = 0,
        RECORDER_RECORDING = 1,
        RECORDER_EDITING = 2
    };

    RecorderState currentState = RECORDER_HOME;

   private:
    NavigationCallback _navCallback;

    Controls *_keyboard;
    Screen *_screen;
    int _selectedIndex = 0;

    FSIO *_fsio;
    AudioResources *_audioResources;
    WavFileWriter *_wavWriter;
    boolean _isRecording = false;
    unsigned long _recordingStartTime = 0;
};

#endif
