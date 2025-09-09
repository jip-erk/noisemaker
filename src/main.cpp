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
#include "hardware/Controls.h"
#include "helper/AudioResources.h"

// U8g2 constructor for SH1106 128x64 I2C display using Wire2
// U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C u8g2(U8G2_R0, /*
// reset=*/U8X8_PIN_NONE);

long oldEncoderValue = 0;

enum AppContext {
    HOME = 0,
    RECORDER = 1,
    LIVE = 2,
};

AppContext currentAppContext;
AppContext lastAppContext;
Screen screen;
Controls controls;

HomeScreen homeContext(&controls, &screen);

void changeContext(AppContext newContext) {
    lastAppContext = currentAppContext;
    currentAppContext = newContext;
}

// AudioResources audioResources;
void sendEventToActiveContext(Controls::ButtonEvent event) {
    switch (currentAppContext) {
        case AppContext::HOME:
            homeContext.handleEvent(event);
            break;
        case AppContext::RECORDER:
            // recorderContext.handleEvent(event);
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

    // Wire2.begin();
    // u8g2.begin();
}

void loop(void) {
    controls.tick();
    // u8g2.clearBuffer();
    // u8g2.setFont(u8g2_font_ncenB08_tr);
    // u8g2.drawStr(0, 20, displayText.c_str());
    // char buf[32];
    // snprintf(buf, sizeof(buf), "Encoder: %ld", newEncoderValue);
    // u8g2.drawStr(0, 50, buf);
    // u8g2.sendBuffer();
}