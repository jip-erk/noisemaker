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
    void checkVolumeThreshold();
    void startWaitingForSound();
    void setVolumeThreshold(float threshold);
    void drawVolumeBar();
    void updateVolumeBar();
    void handleEncoder(int direction);
    void startBuffering();
    void stopBuffering();
    void processAudioBuffer();
    void writeBufferedAudioToFile();

    enum RecorderState {
        RECORDER_HOME = 0,
        RECORDER_WAITING_FOR_SOUND = 1,
        RECORDER_RECORDING = 2,
        RECORDER_EDITING = 3
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
    float _volumeThreshold = 0.1;  // Volume threshold for starting recording
    unsigned long _waitingStartTime = 0;
    float _currentVolume = 0.0;    // Current volume level for display
    
    // Audio buffering for pre-threshold recording
    static const int BUFFER_SIZE = 44100 * 3; // 3 seconds at 44.1kHz
    int16_t _audioBuffer[BUFFER_SIZE];
    int _bufferWriteIndex = 0;
    bool _bufferFull = false;
    bool _isBuffering = false;
};

#endif
