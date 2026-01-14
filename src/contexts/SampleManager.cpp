#include "SampleManager.h"

SampleManager::SampleManager(Controls* keyboard, Screen* screen,
                             NavigationCallback navCallback) {
    _keyboard = keyboard;
    _screen = screen;
    _navCallback = navCallback;
    _managerScreen = SampleManagerScreen(screen);
    _selectedIndex = 0;
    _fileCount = 0;
}

SampleManager::~SampleManager() {}

void SampleManager::refresh() {
    loadFileList();
    _selectedIndex = 0;
    _managerScreen.drawList(_fileList, _selectedIndex, _fileCount);
}

void SampleManager::loadFileList() {
    _fileCount = 0;

    if (!SD.exists("/RECORDINGS")) {
        return;
    }

    File recordingsDir = SD.open("/RECORDINGS");
    if (!recordingsDir || !recordingsDir.isDirectory()) {
        if (recordingsDir) recordingsDir.close();
        return;
    }

    // Read all files
    while (true && _fileCount < MAX_FILES) {
        File entry = recordingsDir.openNextFile();
        if (!entry) break;

        String filename = entry.name();
        if (!entry.isDirectory()) {
             // Filter basic file types if needed, or just show everything
             if (filename.endsWith(".WAV") || filename.endsWith(".wav") || 
                 filename.endsWith(".bdf") || filename.endsWith(".BDF")) { // Show BDF too? Usually hidden. 
                 // Live.cpp hides BDF. Let's show WAVs primarily.
                 if (filename.endsWith(".WAV") || filename.endsWith(".wav")) {
                    _fileList[_fileCount] = filename;
                    _fileCount++;
                 }
             }
        }
        entry.close();
    }
    recordingsDir.close();
}

void SampleManager::deleteSelectedFile() {
    if (_fileCount == 0) return;
    
    String filename = _fileList[_selectedIndex];
    String path = "/RECORDINGS/" + filename;
    
    if (SD.exists(path.c_str())) {
        SD.remove(path.c_str());
        Serial.print("Deleted: ");
        Serial.println(path);
    }
    
    // Refresh list
    loadFileList();
    
    // Adjust index if needed
    if (_selectedIndex >= _fileCount && _fileCount > 0) {
        _selectedIndex = _fileCount - 1;
    }
    // If list is empty, index 0 is fine
}

void SampleManager::handleEvent(Controls::ButtonEvent event) {
    // Encoder - Scroll
    if (event.buttonId == 0 && event.encoderValue != 0) {
        _selectedIndex -= event.encoderValue; // Standard direction
        _selectedIndex = constrain(_selectedIndex, 0, max(0, _fileCount - 1));
        _managerScreen.drawList(_fileList, _selectedIndex, _fileCount);
        return;
    }

    // Button 2 - Back to Home
    if (event.buttonId == 2 && event.state == PRESSED) {
        if (_navCallback) {
            _navCallback(AppContext::HOME);
        }
        return;
    }

    // Button 4 - Delete
    if (event.buttonId == 4 && event.state == PRESSED) {
        // Simple delete confirmation could be added, but request didn't ask for it.
        // Direct delete for now.
        deleteSelectedFile();
        _managerScreen.drawList(_fileList, _selectedIndex, _fileCount);
        return;
    }
}
