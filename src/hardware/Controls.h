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
    static const byte buttonPin1 = 0;
    static const byte buttonPin2 = 1;
    static const byte buttonPin3 = 3;
    static const byte encoderPinA = 2;
    static const byte encoderPinB = 4;

    // Event structure
    struct ButtonEvent {
        uint8_t buttonId;   // 0 for encoder, 1-3 for buttons
        ButtonState state;  // Button state from enum
        long encoderValue;  // Current encoder value

        bool button1Held;
        bool button2Held;
        bool button3Held;
    };

    typedef void (*EventCallback)(ButtonEvent);
    void setEventCallback(EventCallback callback);

    // Check if a button is currently pressed
    bool isDown(uint8_t buttonId);

    bool isComboPressed(uint8_t button1, uint8_t button2);
    bool isComboPressed(uint8_t button1, uint8_t button2, uint8_t button3);

    uint8_t getButtonMask();

   private:
    Encoder encoder;
    long lastEncoderValue;
    EventCallback eventCallback;

    // Button states
    bool button1State, button2State, button3State;
    bool button1LastState, button2LastState, button3LastState;

    // Debounce timing
    static const unsigned long debounceDelay = 5;  // 5ms debounce
    unsigned long button1LastDebounceTime, button2LastDebounceTime,
        button3LastDebounceTime;

    void handleButton(uint8_t buttonId, uint8_t pin, bool& currentState,
                      bool& lastState, unsigned long& lastDebounceTime);

    ButtonEvent createEvent(uint8_t buttonId, ButtonState state,
                            long encoderValue);

    // Reference to self for static callbacks
    static Controls* instance;
};

#endif