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

#include "gui/Screen.h"
#include "gui/screens/HomeScreen.h"
#include "gui/screens/LiveScreen.h"
#include "gui/screens/RecorderScreen.h"
#include "hardware/Controls.h"
#include "helper/AudioResources.h"
#include "helper/FSIO.h"

#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 11
#define SDCARD_SCK_PIN 13
// U8g2 constructor for SH1106 128x64 I2C display using Wire2
// U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C u8g2(U8G2_R0, /*
// reset=*/U8X8_PIN_NONE);

long oldEncoderValue = 0;

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
            homeContext.refresh();
            break;
        case AppContext::RECORDER:
            recorderContext.refresh();
            break;
        case AppContext::LIVE:
            liveContext.refresh();
            break;
        default:
            break;
    }
}

// AudioResources audioResources;
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
    AudioMemory(100);

    audioResources.audioShield.enable();
    delay(100);  // Give time for audio shield to initialize
    audioResources.audioShield.inputSelect(AUDIO_INPUT_MIC);
    audioResources.audioShield.micGain(10);
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
    // Wire2.begin();
    // u8g2.begin();
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

    controls.tick();

    // while (usbMIDI.read()) {
    //     Serial.print("MIDI received - Channel: ");
    //     Serial.print(usbMIDI.getChannel());
    //     Serial.print(" | Type: ");
    //     Serial.print(usbMIDI.getType());
    //     Serial.print(" | Data1: ");
    //     Serial.print(usbMIDI.getData1());
    //     Serial.print(" | Data2: ");
    //     Serial.println(usbMIDI.getData2());
    // }
}