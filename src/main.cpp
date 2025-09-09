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
#include "hardware/Controls.h"
#include "helper/AudioResources.h"
#include "helper/FSIO.h"
#include "helper/SampleFSIO.h"

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
            // liveContext.handleEvent(event);
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
            // liveContext.handleEvent(event);
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

    screen.begin();
    controls.setEventCallback(handleControlEvent);

    currentAppContext = AppContext::HOME;
    homeContext.refresh();
    // Wire2.begin();
    // u8g2.begin();
}

void loop(void) { controls.tick(); }