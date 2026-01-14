#ifndef SampleManager_h
#define SampleManager_h

#include <Arduino.h>
#include <SD.h>

#include "../gui/Screen.h"
#include "../gui/screens/SampleManagerScreen.h"
#include "../hardware/Controls.h"
#include "../main.h"

class SampleManager {
   public:
    typedef void (*NavigationCallback)(AppContext newContext);

    SampleManager(Controls* keyboard, Screen* screen,
                  NavigationCallback navCallback = nullptr);
    ~SampleManager();

    void refresh();
    void handleEvent(Controls::ButtonEvent);

   private:
    NavigationCallback _navCallback;
    Controls* _keyboard;
    Screen* _screen;
    SampleManagerScreen _managerScreen;

    static const int MAX_FILES = 50;
    String _fileList[MAX_FILES];
    int _fileCount = 0;
    int _selectedIndex = 0;

    void loadFileList();
    void deleteSelectedFile();
};

#endif
