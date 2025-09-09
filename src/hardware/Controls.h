#ifndef Controls_h
#define Controls_h

#include <Arduino.h>
#include <Encoder.h>
#include <OneButton.h>

// Button state enum
enum ButtonState { NOT_PRESSED = 0, PRESSED = 1, LONG_PRESSED = 2 };

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
    };

    typedef void (*EventCallback)(ButtonEvent);
    void setEventCallback(EventCallback callback);

   private:
    OneButton button1;
    OneButton button2;
    OneButton button3;
    Encoder encoder;
    long lastEncoderValue;
    EventCallback eventCallback;

    // Static callback handlers
    static void handleButton1Click();
    static void handleButton1LongPress();

    static void handleButton2Click();
    static void handleButton2LongPress();

    static void handleButton3Click();
    static void handleButton3LongPress();

    // Reference to self for static callbacks
    static Controls* instance;
};

#endif