#include "Controls.h"

Controls* Controls::instance = nullptr;

// LED pin mapping for buttons 0-5: (0=encoder, 1-5=buttons)
const int Controls::ledPinMap[7] = {-1,
                                    Controls::ledPin1,
                                    Controls::ledPin2,
                                    Controls::ledPin3,
                                    Controls::ledPin4,
                                    -1,
                                    -1};

Controls::Controls()
    : encoder(encoderPinA, encoderPinB),
      lastEncoderValue(0),
      eventCallback(nullptr) {
    instance = this;

    // Setup pins
    pinMode(buttonPin1, INPUT_PULLUP);
    pinMode(buttonPin2, INPUT_PULLUP);
    pinMode(buttonPin3, INPUT_PULLUP);
    pinMode(buttonPin4, INPUT_PULLUP);
    pinMode(buttonPin5, INPUT_PULLUP);

    pinMode(ledPin1, OUTPUT);
    pinMode(ledPin2, OUTPUT);
    pinMode(ledPin3, OUTPUT);
    pinMode(ledPin4, OUTPUT);

    // Initialize button states
    button1State = false;
    button2State = false;
    button3State = false;
    button4State = false;
    button5State = false;

    button1LastState = false;
    button2LastState = false;
    button3LastState = false;
    button4LastState = false;
    button5LastState = false;

    // Initialize debounce times
    button1LastDebounceTime = 0;
    button2LastDebounceTime = 0;
    button3LastDebounceTime = 0;
    button4LastDebounceTime = 0;
    button5LastDebounceTime = 0;

    // Setup LED pins as outputs

    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, LOW);
}

void Controls::tick() {
    // Handle encoder
    long newEncoderValue = encoder.read() / 4;
    if (newEncoderValue != lastEncoderValue) {
        int direction = 0;
        if (newEncoderValue > lastEncoderValue) {
            direction = 1;  // Forward
        } else if (newEncoderValue < lastEncoderValue) {
            direction = -1;  // Back
        }

        ButtonEvent event = createEvent(0, NOT_PRESSED, direction);
        if (eventCallback) eventCallback(event);
        lastEncoderValue = newEncoderValue;
    }

    // Handle buttons
    handleButton(1, buttonPin1, button1State, button1LastState,
                 button1LastDebounceTime);
    handleButton(2, buttonPin2, button2State, button2LastState,
                 button2LastDebounceTime);
    handleButton(3, buttonPin3, button3State, button3LastState,
                 button3LastDebounceTime);
    handleButton(4, buttonPin4, button4State, button4LastState,
                 button4LastDebounceTime);
    handleButton(5, buttonPin5, button5State, button5LastState,
                 button5LastDebounceTime);
}

void Controls::handleButton(uint8_t buttonId, uint8_t pin, bool& currentState,
                            bool& lastState, unsigned long& lastDebounceTime) {
    bool reading = !digitalRead(pin);  // Inverted because of INPUT_PULLUP
    unsigned long currentTime = millis();

    // Debouncing
    if (reading != lastState) {
        lastDebounceTime = currentTime;
    }

    if ((currentTime - lastDebounceTime) > debounceDelay) {
        // Button state has stabilized
        if (reading != currentState) {
            currentState = reading;

            // Send event on both press and release with current button states
            ButtonState state = currentState ? PRESSED : NOT_PRESSED;
            ButtonEvent event = createEvent(buttonId, state, 0);
            if (eventCallback) eventCallback(event);
        }
    }

    lastState = reading;
}

Controls::ButtonEvent Controls::createEvent(uint8_t buttonId, ButtonState state,
                                            long encoderValue) {
    ButtonEvent event;
    event.buttonId = buttonId;
    event.state = state;
    event.encoderValue = encoderValue;

    // Include current state of all buttons
    event.button1Held = button1State;
    event.button2Held = button2State;
    event.button3Held = button3State;
    event.button4Held = button4State;
    event.button5Held = button5State;

    return event;
}

void Controls::setEventCallback(EventCallback callback) {
    eventCallback = callback;
}

bool Controls::isDown(uint8_t buttonId) {
    switch (buttonId) {
        case 1:
            return button1State;
        case 2:
            return button2State;
        case 3:
            return button3State;
        case 4:
            return button4State;
        case 5:
            return button5State;
        default:
            return false;
    }
}

bool Controls::isComboPressed(uint8_t btn1, uint8_t btn2) {
    return isDown(btn1) && isDown(btn2);
}

bool Controls::isComboPressed(uint8_t btn1, uint8_t btn2, uint8_t btn3) {
    return isDown(btn1) && isDown(btn2) && isDown(btn3);
}

void Controls::triggerLedForButton(uint8_t buttonId, bool isPressed) {
    if (buttonId < 7 && ledPinMap[buttonId] >= 0) {
        digitalWrite(ledPinMap[buttonId], isPressed ? HIGH : LOW);
    }
}

void Controls::triggerLedForButton(uint8_t buttonId, uint8_t brightness) {
    if (buttonId < 7 && ledPinMap[buttonId] >= 0) {
        analogWrite(ledPinMap[buttonId], brightness);
    }
}