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
#include "gui/screens/RecorderScreen.h"
#include "gui/screens/LiveScreen.h"
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

    // Mute/unmute microphone based on context
    if (currentAppContext == AppContext::LIVE) {
        // Mute mic input in live context - ALWAYS muted
        audioResources.mixer1.gain(1, 0.0); // Input channel muted
        audioResources.mixer1.gain(0, 1.0); // Playback channel active
    } else {
        // Unmute mic input in other contexts
        audioResources.mixer1.gain(1, 1.0); // Input channel active
        audioResources.mixer1.gain(0, 0.0); // Playback channel muted
    }

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
    // USB audio doesn't use audioShield volume control - volume controlled by host

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

    // Set up audio resources for recorder and live screen
    recorderContext.setAudioResources(&audioResources);
    liveContext.setAudioResources(&audioResources);

    currentAppContext = AppContext::HOME;
    homeContext.refresh();
    // Wire2.begin();
    // u8g2.begin();
}

void loop(void) {
    // Only update recorder context when it's the active context
    if (currentAppContext == AppContext::RECORDER) {
        if (recorderContext.currentState == recorderContext.RECORDER_RECORDING)
            recorderContext.continueRecording();
        else if (recorderContext.currentState ==
                 recorderContext.RECORDER_WAITING_FOR_SOUND)
            recorderContext.checkVolumeThreshold();
        else if (recorderContext.currentState ==
                 recorderContext.RECORDER_HOME) {
            // Update volume bar on home screen
            recorderContext.updateVolumeBar();
        }
    }
    
    // Update live context when it's the active context
    if (currentAppContext == AppContext::LIVE) {
        if (liveContext.currentState == liveContext.LIVE_PLAYING || 
            liveContext.currentState == liveContext.LIVE_PAUSED) {
            liveContext.updatePlayback();
        }
    }

    controls.tick();
}