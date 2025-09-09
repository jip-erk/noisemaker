#include "Controls.h"

Controls* Controls::instance = nullptr;

Controls::Controls()
    : button1(
          buttonPin1, true,
          false),  // Changed third parameter to false to disable double-click
      button2(buttonPin2, true, false),
      button3(buttonPin3, true, false),
      encoder(encoderPinA, encoderPinB),
      lastEncoderValue(0),
      eventCallback(nullptr) {
    instance = this;

    // Setup pins
    pinMode(buttonPin1, INPUT_PULLUP);
    pinMode(buttonPin2, INPUT_PULLUP);
    pinMode(buttonPin3, INPUT_PULLUP);

    // Initialize buttons with faster response times
    button1.setDebounceMs(5);  // 5ms debounce
    button2.setDebounceMs(5);
    button3.setDebounceMs(5);

    // Initialize buttons
    button1.reset();
    button2.reset();
    button3.reset();

    // Initialize button callbacks
    button1.attachClick(handleButton1Click);
    button1.attachLongPressStart(handleButton1LongPress);

    button2.attachClick(handleButton2Click);
    button2.attachLongPressStart(handleButton2LongPress);

    button3.attachClick(handleButton3Click);
    button3.attachLongPressStart(handleButton3LongPress);
}

void Controls::tick() {
    button1.tick();
    button2.tick();
    button3.tick();

    long newEncoderValue = encoder.read() / 4;
    if (newEncoderValue != lastEncoderValue) {
        int direction = 0;
        if (newEncoderValue > lastEncoderValue) {
            direction = 1;  // Forward
        } else if (newEncoderValue < lastEncoderValue) {
            direction = -1;  // Back
        }

        ButtonEvent event = {0, NOT_PRESSED, direction};
        if (eventCallback) eventCallback(event);
        lastEncoderValue = newEncoderValue;
    }
}

void Controls::setEventCallback(EventCallback callback) {
    eventCallback = callback;
}
// Button 1 handlers
void Controls::handleButton1Click() {
    if (instance && instance->eventCallback) {
        ButtonEvent event = {1, PRESSED, 0};
        instance->eventCallback(event);
    }
}

void Controls::handleButton1LongPress() {
    if (instance && instance->eventCallback) {
        ButtonEvent event = {1, LONG_PRESSED, 0};
        instance->eventCallback(event);
    }
}

// Button 2 handlers
void Controls::handleButton2Click() {
    if (instance && instance->eventCallback) {
        ButtonEvent event = {2, PRESSED, 0};
        instance->eventCallback(event);
    }
}

void Controls::handleButton2LongPress() {
    if (instance && instance->eventCallback) {
        ButtonEvent event = {2, LONG_PRESSED, 0};
        instance->eventCallback(event);
    }
}

// Button 3 handlers
void Controls::handleButton3Click() {
    if (instance && instance->eventCallback) {
        ButtonEvent event = {3, PRESSED, 0};
        instance->eventCallback(event);
    }
}

void Controls::handleButton3LongPress() {
    if (instance && instance->eventCallback) {
        ButtonEvent event = {3, LONG_PRESSED, 0};
        instance->eventCallback(event);
    }
}