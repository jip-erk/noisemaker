#ifndef Controls_h
#define Controls_h

#include <Arduino.h>
#include <Encoder.h>

// Button state enum
enum ButtonState { NOT_PRESSED = 0, PRESSED = 1 };

class Controls {
   public:
    Controls();
    void tick();

    // Button pins
    static const byte buttonPin1 = 4;
    static const byte buttonPin2 = 6;
    static const byte buttonPin3 = 5;
    static const byte buttonPin4 = 3;
    static const byte buttonPin5 = 7;
    static const byte encoderPinA = 1;
    static const byte encoderPinB = 0;

    // LED pins
    static const byte ledPin1 = 2;
    static const byte ledPin2 = 15;
    static const byte ledPin3 = 13;
    static const byte ledPin4 = 9;

    // Event structure
    struct ButtonEvent {
        uint8_t buttonId;   // 0 for encoder rotation, 1-5 for buttons
        ButtonState state;  // Button state from enum
        long encoderValue;  // Current encoder value

        bool button1Held;
        bool button2Held;
        bool button3Held;
        bool button4Held;
        bool button5Held;
    };

    typedef void (*EventCallback)(ButtonEvent);
    void setEventCallback(EventCallback callback);

    // Check if a button is currently pressed
    bool isDown(uint8_t buttonId);

    bool isComboPressed(uint8_t button1, uint8_t button2);
    bool isComboPressed(uint8_t button1, uint8_t button2, uint8_t button3);

    // LED management
    void triggerLedForButton(uint8_t buttonId, bool isPressed);

   private:
    Encoder encoder;
    long lastEncoderValue;
    EventCallback eventCallback;

    // LED pin mapping (7 possible button IDs: 0-6)
    static const int ledPinMap[7];

    // Button states
    bool button1State, button2State, button3State, button4State, button5State,
        encoderButtonState;
    bool button1LastState, button2LastState, button3LastState, button4LastState,
        button5LastState, encoderButtonLastState;

    // Debounce timing
    static const unsigned long debounceDelay = 25;  // 25ms debounce
    unsigned long button1LastDebounceTime, button2LastDebounceTime,
        button3LastDebounceTime, button4LastDebounceTime,
        button5LastDebounceTime, encoderButtonLastDebounceTime;

    void handleButton(uint8_t buttonId, uint8_t pin, bool& currentState,
                      bool& lastState, unsigned long& lastDebounceTime);

    ButtonEvent createEvent(uint8_t buttonId, ButtonState state,
                            long encoderValue);

    // Reference to self for static callbacks
    static Controls* instance;
};

#endif