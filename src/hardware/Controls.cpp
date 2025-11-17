#include "Controls.h"

Controls* Controls::instance = nullptr;

Controls::Controls()
    : encoder(encoderPinA, encoderPinB),
      lastEncoderValue(0),
      eventCallback(nullptr) {
    instance = this;

    // Setup pins
    pinMode(buttonPin1, INPUT_PULLUP);
    pinMode(buttonPin2, INPUT_PULLUP);
    pinMode(buttonPin3, INPUT_PULLUP);

    // Initialize button states
    button1State = false;
    button2State = false;
    button3State = false;

    button1LastState = false;
    button2LastState = false;
    button3LastState = false;

    // Initialize debounce times
    button1LastDebounceTime = 0;
    button2LastDebounceTime = 0;
    button3LastDebounceTime = 0;
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

uint8_t Controls::getButtonMask() {
    uint8_t mask = 0;
    if (button1State) mask |= (1 << 0);  // Bit 0
    if (button2State) mask |= (1 << 1);  // Bit 1
    if (button3State) mask |= (1 << 2);  // Bit 2
    return mask;
}

// Usage Examples:

void onControlEvent(Controls::ButtonEvent event) {
    // Example 1: Encoder with button 1 held = fine adjustment
    if (event.buttonId == 0 && event.encoderValue != 0) {
        if (event.button1Held) {
            Serial.printf("Fine tune: %d\n", event.encoderValue);
        } else {
            Serial.printf("Normal scroll: %d\n", event.encoderValue);
        }
    }

    // Example 2: Encoder with button 2 held = zoom
    if (event.buttonId == 0 && event.button2Held) {
        if (event.encoderValue > 0) {
            Serial.println("Zoom in");
        } else {
            Serial.println("Zoom out");
        }
    }

    // Example 3: Button 1 pressed while button 2 is held
    if (event.buttonId == 1 && event.state == PRESSED && event.button2Held) {
        Serial.println("Button 1+2 combo!");
    }

    // Example 4: Three button combo
    if (event.button1Held && event.button2Held && event.button3Held) {
        Serial.println("All three buttons held!");
    }

    // Example 5: Check specific combinations
    if (event.buttonId == 0) {  // Encoder event
        uint8_t mask = (event.button1Held ? 1 : 0) |
                       (event.button2Held ? 2 : 0) |
                       (event.button3Held ? 4 : 0);

        switch (mask) {
            case 0b001:  // Only button 1
                Serial.println("Encoder + Button1");
                break;
            case 0b010:  // Only button 2
                Serial.println("Encoder + Button2");
                break;
            case 0b011:  // Button 1 + 2
                Serial.println("Encoder + Button1&2");
                break;
            case 0b111:  // All three
                Serial.println("Encoder + All buttons");
                break;
        }
    }
}