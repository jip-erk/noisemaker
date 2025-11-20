#ifndef LiveScreen_h
#define LiveScreen_h

#include <Arduino.h>
#include <SD.h>

#include "../../hardware/Controls.h"
#include "../../helper/AudioResources.h"
#include "../../helper/SampleSlot.hpp"
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
        LIVE_SLOT_VIEW = 0,      // Viewing/selecting slots
        LIVE_SAMPLE_SELECT = 1,  // Selecting a sample for a slot
        LIVE_PLAYING = 2         // Currently playing samples
    };

    LiveState currentState = LIVE_SLOT_VIEW;

   private:
    NavigationCallback _navCallback;
    Controls *_keyboard;
    Screen *_screen;
    AudioResources *_audioResources;

    // Sample slot management
    static const int NUM_SLOTS = 4;
    SampleSlot _slots[NUM_SLOTS];
    int _selectedSlotIndex = 0;

    // Default MIDI note mappings (C3, D3, E3, F3)
    static const uint8_t DEFAULT_MIDI_NOTES[NUM_SLOTS];

    // Slot labels for display
    static const char* SLOT_LABELS[NUM_SLOTS];

    // File list for sample selection
    int _selectedFileIndex = 0;
    int _fileCount = 0;
    String _fileList[20];  // Max 20 files

    // UI functions
    void drawSlotView();
    void drawSampleSelect();

    // Sample management
    void loadFileList();
    void assignSampleToSlot();
    void clearSlot(int slotIndex);

    // Playback functions
    void playSlot(int slotIndex);
    void stopSlot(int slotIndex);
    void stopAllSlots();

    // Helper to extract filename without extension
    String getFileNameWithoutExtension(const String& fileName);
};

#endif
