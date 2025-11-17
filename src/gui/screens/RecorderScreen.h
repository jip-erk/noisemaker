#ifndef RecorderScreen_h
#define RecorderScreen_h

#include <Arduino.h>
#include <SD.h>

#include "../../hardware/Controls.h"
#include "../../helper/AudioResources.h"
#include "../../helper/FSIO.h"
#include "../../helper/NameGenerator.hpp"
#include "../../helper/WavFileWriter.hpp"
#include "../../main.h"
#include "../Screen.h"
#include "components/TextHeader.h"
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
    void writeBufferedAudioToFile();

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
    TextHeader _header;
    VolumeBar _volumeBar;
    Waveform _waveform;
    WaveformSelector _waveformSelector;
    Controls* _keyboard;
    Screen* _screen;
    int _selectedIndex = 0;

    FSIO* _fsio;
    AudioResources* _audioResources;
    WavFileWriter* _wavWriter;
    boolean _isRecording = false;
    unsigned long _recordingStartTime = 0;
    float _volumeThreshold = 0.1;  // Volume threshold for starting recording
    unsigned long _waitingStartTime = 0;
    String _recordedFileName;

    // Audio buffering for pre-threshold recording
    static const int BUFFER_SIZE = 44100 * 3;  // 3 seconds at 44.1kHz
    int16_t _audioBuffer[BUFFER_SIZE];
    int _bufferWriteIndex = 0;
    bool _bufferFull = false;
    bool _isBuffering = false;
    NameGenerator gen;
};

#endif
