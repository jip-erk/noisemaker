#ifndef RecorderScreen_h
#define RecorderScreen_h

#include <Arduino.h>

#include "../../hardware/Controls.h"
#include "../../helper/AudioResources.h"
#include "../../helper/FSIO.h"
#include "../../helper/SampleFSIO.h"
#include "../../main.h"
#include "../Screen.h"

class RecorderScreen {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);

    RecorderScreen(Controls *keyboard, Screen *screen,
                   NavigationCallback navCallback = nullptr);

    void handleEvent(Controls::ButtonEvent);
    void refresh();
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
    SampleFSIO *_sfsio;
    AudioResources *_audioResources;
    boolean _isRecording = false;
    File _frec;
};

#endif
