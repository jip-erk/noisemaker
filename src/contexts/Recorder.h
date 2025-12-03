#ifndef Recorder_h
#define Recorder_h

#include <Arduino.h>
#include <SD.h>

#include "../hardware/Controls.h"
#include "../helper/AudioResources.h"
#include "../helper/NameGenerator.hpp"
#include "../helper/WavFileWriter.hpp"
#include "../main.h"
#include "../gui/Screen.h"
#include "../gui/screens/RecorderScreen.h"

class Recorder {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);

    Recorder(Controls *keyboard, Screen *screen, NavigationCallback navCallback = nullptr);
    ~Recorder();

    void refresh();
    long receiveTimerTick();
    void handleEvent(Controls::ButtonEvent);
    void setAudioResources(AudioResources *audioResources);

    void continueRecording();

    enum RecorderState {
        RECORDER_HOME = 0,
        RECORDER_RECORDING = 1,
        RECORDER_EDITING = 2
    };

    static String getFilePath(const String &fileName) {
        return "/RECORDINGS/" + fileName + ".wav";
    }

    RecorderState currentState = RECORDER_HOME;

   private:
    NavigationCallback _navCallback;
    Controls *_keyboard;
    Screen *_screen;
    RecorderScreen _recorderScreen;

    AudioResources *_audioResources;
    WavFileWriter *_wavWriter;
    unsigned long _recordingStartTime = 0;
    String _recordedFileName;
    NameGenerator gen;

    // Private methods - business logic
    void startRecording();
    void stopRecording();
    void showRecorderScreen();
    void showEditScreen();
    void updateWaveform();
    void updateVolumeBar();
    void saveBinaryDataFile(const String &fileName, uint32_t startPos, uint32_t endPos);
};

#endif
