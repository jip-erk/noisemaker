#ifndef LiveScreen_h
#define LiveScreen_h

#include <Arduino.h>
#include <SD.h>

#include "../../hardware/Controls.h"
#include "../../helper/AudioResources.h"
#include "../../main.h"
#include "../Screen.h"

class LiveScreen {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);

    LiveScreen(Controls *keyboard, Screen *screen,
               NavigationCallback navCallback = nullptr);
    ~LiveScreen();

    void handleEvent(Controls::ButtonEvent);
    void refresh();
    void setAudioResources(AudioResources* audioResources);

    enum LiveState {
        LIVE_HOME = 0,
        LIVE_PLAYING = 1,
        LIVE_PAUSED = 2
    };

    LiveState currentState = LIVE_HOME;

   private:
    NavigationCallback _navCallback;

    Controls *_keyboard;
    Screen *_screen;
    AudioResources *_audioResources;
    
    int _selectedIndex = 0;
    int _fileCount = 0;
    String _fileList[20]; // Max 20 files
    String _currentPlayingFile = "";
    unsigned long _playbackStartTime = 0;
    
    void loadFileList();
    void playSelectedFile();
    void stopPlayback();
    void updatePlayback();
    void drawFileList();
    void drawPlaybackInfo();
};

#endif
