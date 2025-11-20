#include "main.h"

#include <Arduino.h>
#include <Audio.h>
#include <Encoder.h>  // Add this line
#include <OneButton.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <usb_midi.h>

#include "gui/Screen.h"
#include "gui/screens/HomeScreen.h"
#include "gui/screens/LiveScreen.h"
#include "gui/screens/RecorderScreen.h"
#include "hardware/Controls.h"
#include "helper/AudioResources.h"

#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 11
#define SDCARD_SCK_PIN 13

// Audio configuration constants
#define AUDIO_MEMORY_BLOCKS 100
#define AUDIO_SHIELD_INIT_DELAY_MS 100
#define DEFAULT_MIC_GAIN 10

// Interval Timer for playback pattern, metronome, etc..
IntervalTimer globalTickTimer;
volatile bool ticked = false;
long globalTickInterval = 1000000;  // intervall in microseconds -> starts at 1s
long globalTickIntervalNew = 1000000;
void globalTick();

AppContext currentAppContext;
AppContext lastAppContext;

Screen screen;
Controls controls;

void changeContext(AppContext newContext);

HomeScreen homeContext(&controls, &screen, changeContext);
RecorderScreen recorderContext(&controls, &screen, changeContext);
LiveScreen liveContext(&controls, &screen, changeContext);
AudioResources audioResources;

void changeContext(AppContext newContext) {
    lastAppContext = currentAppContext;
    currentAppContext = newContext;

    switch (currentAppContext) {
        case AppContext::HOME:
            audioResources.disableLivePassthrough();
            homeContext.refresh();
            break;
        case AppContext::RECORDER:
            audioResources.disableLivePassthrough();
            recorderContext.refresh();
            break;
        case AppContext::LIVE:
            audioResources.disableLivePassthrough();
            liveContext.refresh();
            break;
        default:
            break;
    }
}

void sendEventToActiveContext(Controls::ButtonEvent event) {
    switch (currentAppContext) {
        case AppContext::HOME:
            homeContext.handleEvent(event);
            break;
        case AppContext::RECORDER:
            recorderContext.handleEvent(event);
            break;
        case AppContext::LIVE:
            liveContext.handleEvent(event);
            break;
        default:
            break;
    }
}

void handleControlEvent(Controls::ButtonEvent event) {
    sendEventToActiveContext(event);
}

void setup(void) {
    Serial.begin(9600);

    AudioMemoryUsageMaxReset();
    AudioMemory(AUDIO_MEMORY_BLOCKS);

    audioResources.audioShield.enable();
    delay(AUDIO_SHIELD_INIT_DELAY_MS);
    audioResources.audioShield.inputSelect(AUDIO_INPUT_MIC);
    audioResources.audioShield.micGain(DEFAULT_MIC_GAIN);
    // USB audio doesn't use audioShield volume control - volume controlled by
    // host

    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
    if (!(SD.begin(SDCARD_CS_PIN))) {
        // stop here if no SD card, but print a message
        while (1) {
            Serial.println("Unable to access the SD card");
            Serial.println(SDCARD_CS_PIN);
            delay(500);
        }
    }

    screen.begin();
    controls.setEventCallback(handleControlEvent);

    globalTickTimer.begin(globalTick, globalTickInterval);

    // Set up audio resources for recorder and live screen
    recorderContext.setAudioResources(&audioResources);
    liveContext.setAudioResources(&audioResources);

    currentAppContext = AppContext::HOME;
    homeContext.refresh();
}

void globalTick() { ticked = true; }

void updateTickInterval(long newInterval) {
    // Only update if interval has actually changed and is valid
    if (newInterval > 0 && newInterval != globalTickInterval) {
        noInterrupts();  // Disable interrupts while updating timer
        globalTickInterval = newInterval;
        globalTickTimer.update(globalTickInterval);
        interrupts();  // Re-enable interrupts
    }
}

void sendTickToActiveContext() {
    switch (currentAppContext) {
        case AppContext::RECORDER:
            globalTickIntervalNew = recorderContext.receiveTimerTick();
            updateTickInterval(globalTickIntervalNew);
            break;
        default:
            break;
    }
}

void loop(void) {
    if (recorderContext.currentState == recorderContext.RECORDER_RECORDING)
        recorderContext.continueRecording();

    if (ticked) {
        ticked = false;
        sendTickToActiveContext();
    }

    // Handle MIDI input (only in LIVE mode)
    if (currentAppContext == AppContext::LIVE) {
        while (usbMIDI.read()) {
            byte type = usbMIDI.getType();
            if (type == usbMIDI.NoteOn) {
                byte note = usbMIDI.getData1();
                byte velocity = usbMIDI.getData2();
                liveContext.handleMidiNote(note, velocity);
            }
        }
    }

    controls.tick();
}