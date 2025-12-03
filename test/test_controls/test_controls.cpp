#include <Arduino.h>
#include <unity.h>

// Button state enum (from Controls.h)
enum ButtonState { NOT_PRESSED = 0, PRESSED = 1 };

// Test ButtonEvent structure
struct ButtonEvent {
    uint8_t buttonId;
    ButtonState state;
    long encoderValue;
    bool button1Held;
    bool button2Held;
    bool button3Held;
};

// Helper functions to test button logic
class ControlsTest {
public:
    // Test debounce logic
    static bool shouldButtonStateChange(bool currentState, bool readState,
                                        unsigned long lastDebounceTime,
                                        unsigned long currentTime,
                                        unsigned long debounceDelay = 5) {
        if (readState != currentState) {
            if ((currentTime - lastDebounceTime) > debounceDelay) {
                return true;
            }
        }
        return false;
    }

    // Create a button event (mimics Controls::createEvent)
    static ButtonEvent createEvent(uint8_t buttonId, ButtonState state,
                                   long encoderValue, bool button1Held,
                                   bool button2Held, bool button3Held) {
        ButtonEvent event;
        event.buttonId = buttonId;
        event.state = state;
        event.encoderValue = encoderValue;
        event.button1Held = button1Held;
        event.button2Held = button2Held;
        event.button3Held = button3Held;
        return event;
    }

    // Test button combo detection
    static bool isComboPressed(bool button1, bool button2, bool button3) {
        return button1 && button2 && button3;
    }

    static bool isComboPressed(bool button1, bool button2) {
        return button1 && button2;
    }

    // Get button mask for all pressed buttons
    static uint8_t getButtonMask(bool button1, bool button2, bool button3) {
        return (button1 ? 0x01 : 0x00) | (button2 ? 0x02 : 0x00) |
               (button3 ? 0x04 : 0x00);
    }
};

void setUp(void) {}

void tearDown(void) {}

// Test button event creation with all fields
void test_button_event_creation(void) {
    ButtonEvent event = ControlsTest::createEvent(1, PRESSED, 0, false, false, false);

    TEST_ASSERT_EQUAL_UINT8(1, event.buttonId);
    TEST_ASSERT_EQUAL_INT(PRESSED, event.state);
    TEST_ASSERT_EQUAL_INT(0, event.encoderValue);
}

// Test button event carries encoder value
void test_button_event_with_encoder_value(void) {
    ButtonEvent event =
        ControlsTest::createEvent(0, NOT_PRESSED, 42, false, false, false);

    TEST_ASSERT_EQUAL_INT(42, event.encoderValue);
}

// Test button event carries combo state
void test_button_event_combo_state(void) {
    ButtonEvent event =
        ControlsTest::createEvent(1, PRESSED, 0, true, true, false);

    TEST_ASSERT_TRUE(event.button1Held);
    TEST_ASSERT_TRUE(event.button2Held);
    TEST_ASSERT_FALSE(event.button3Held);
}

// Test debounce prevents rapid state changes
void test_debounce_prevents_bounce(void) {
    bool currentState = false;
    bool readState = true;
    unsigned long lastDebounceTime = 100;
    unsigned long currentTime = 102;  // Only 2ms later (< 5ms debounce)

    bool shouldChange = ControlsTest::shouldButtonStateChange(
        currentState, readState, lastDebounceTime, currentTime, 5);

    TEST_ASSERT_FALSE(shouldChange);
}

// Test debounce allows state change after delay
void test_debounce_allows_change_after_delay(void) {
    bool currentState = false;
    bool readState = true;
    unsigned long lastDebounceTime = 100;
    unsigned long currentTime = 106;  // 6ms later (> 5ms debounce)

    bool shouldChange = ControlsTest::shouldButtonStateChange(
        currentState, readState, lastDebounceTime, currentTime, 5);

    TEST_ASSERT_TRUE(shouldChange);
}

// Test debounce with exact timing boundary
void test_debounce_boundary_condition(void) {
    bool currentState = false;
    bool readState = true;
    unsigned long lastDebounceTime = 100;
    unsigned long currentTime = 105;  // Exactly 5ms later

    bool shouldChange = ControlsTest::shouldButtonStateChange(
        currentState, readState, lastDebounceTime, currentTime, 5);

    TEST_ASSERT_TRUE(shouldChange);
}

// Test no state change if read value equals current state
void test_no_change_if_stable(void) {
    bool currentState = true;
    bool readState = true;
    unsigned long lastDebounceTime = 100;
    unsigned long currentTime = 200;

    bool shouldChange = ControlsTest::shouldButtonStateChange(
        currentState, readState, lastDebounceTime, currentTime, 5);

    TEST_ASSERT_FALSE(shouldChange);
}

// Test two-button combo detection
void test_two_button_combo_pressed(void) {
    bool combo = ControlsTest::isComboPressed(true, true);
    TEST_ASSERT_TRUE(combo);
}

// Test two-button combo not pressed when only one button down
void test_two_button_combo_one_button(void) {
    bool combo = ControlsTest::isComboPressed(true, false);
    TEST_ASSERT_FALSE(combo);
}

// Test two-button combo not pressed when no buttons down
void test_two_button_combo_no_buttons(void) {
    bool combo = ControlsTest::isComboPressed(false, false);
    TEST_ASSERT_FALSE(combo);
}

// Test three-button combo detection
void test_three_button_combo_pressed(void) {
    bool combo = ControlsTest::isComboPressed(true, true, true);
    TEST_ASSERT_TRUE(combo);
}

// Test three-button combo fails with missing one button
void test_three_button_combo_missing_one(void) {
    bool combo = ControlsTest::isComboPressed(true, true, false);
    TEST_ASSERT_FALSE(combo);
}

// Test three-button combo with only two buttons
void test_three_button_combo_two_buttons(void) {
    bool combo = ControlsTest::isComboPressed(true, false, true);
    TEST_ASSERT_FALSE(combo);
}

// Test button mask for single button
void test_button_mask_single_button(void) {
    uint8_t mask = ControlsTest::getButtonMask(true, false, false);
    TEST_ASSERT_EQUAL_UINT8(0x01, mask);
}

// Test button mask for two buttons
void test_button_mask_two_buttons(void) {
    uint8_t mask = ControlsTest::getButtonMask(true, true, false);
    TEST_ASSERT_EQUAL_UINT8(0x03, mask);
}

// Test button mask for all three buttons
void test_button_mask_all_buttons(void) {
    uint8_t mask = ControlsTest::getButtonMask(true, true, true);
    TEST_ASSERT_EQUAL_UINT8(0x07, mask);
}

// Test button mask for no buttons
void test_button_mask_no_buttons(void) {
    uint8_t mask = ControlsTest::getButtonMask(false, false, false);
    TEST_ASSERT_EQUAL_UINT8(0x00, mask);
}

// Test button mask bits are unique
void test_button_mask_button2_bit(void) {
    uint8_t mask = ControlsTest::getButtonMask(false, true, false);
    TEST_ASSERT_EQUAL_UINT8(0x02, mask);
}

// Test button mask bits are unique for button 3
void test_button_mask_button3_bit(void) {
    uint8_t mask = ControlsTest::getButtonMask(false, false, true);
    TEST_ASSERT_EQUAL_UINT8(0x04, mask);
}

// Test encoder value is zero by default
void test_encoder_default_value(void) {
    ButtonEvent event =
        ControlsTest::createEvent(0, NOT_PRESSED, 0, false, false, false);
    TEST_ASSERT_EQUAL_INT(0, event.encoderValue);
}

// Test encoder value preserves negative values
void test_encoder_negative_value(void) {
    ButtonEvent event =
        ControlsTest::createEvent(0, NOT_PRESSED, -10, false, false, false);
    TEST_ASSERT_EQUAL_INT(-10, event.encoderValue);
}

// Test encoder value preserves large positive values
void test_encoder_large_positive_value(void) {
    ButtonEvent event =
        ControlsTest::createEvent(0, NOT_PRESSED, 500, false, false, false);
    TEST_ASSERT_EQUAL_INT(500, event.encoderValue);
}

// Test button ID range for valid buttons
void test_button_id_valid_range(void) {
    for (uint8_t id = 0; id <= 3; id++) {
        ButtonEvent event = ControlsTest::createEvent(id, NOT_PRESSED, 0,
                                                      false, false, false);
        TEST_ASSERT_EQUAL_UINT8(id, event.buttonId);
    }
}

int main(int argc, char** argv) {
    UNITY_BEGIN();

    // Button event tests
    RUN_TEST(test_button_event_creation);
    RUN_TEST(test_button_event_with_encoder_value);
    RUN_TEST(test_button_event_combo_state);

    // Debounce tests
    RUN_TEST(test_debounce_prevents_bounce);
    RUN_TEST(test_debounce_allows_change_after_delay);
    RUN_TEST(test_debounce_boundary_condition);
    RUN_TEST(test_no_change_if_stable);

    // Two-button combo tests
    RUN_TEST(test_two_button_combo_pressed);
    RUN_TEST(test_two_button_combo_one_button);
    RUN_TEST(test_two_button_combo_no_buttons);

    // Three-button combo tests
    RUN_TEST(test_three_button_combo_pressed);
    RUN_TEST(test_three_button_combo_missing_one);
    RUN_TEST(test_three_button_combo_two_buttons);

    // Button mask tests
    RUN_TEST(test_button_mask_single_button);
    RUN_TEST(test_button_mask_two_buttons);
    RUN_TEST(test_button_mask_all_buttons);
    RUN_TEST(test_button_mask_no_buttons);
    RUN_TEST(test_button_mask_button2_bit);
    RUN_TEST(test_button_mask_button3_bit);

    // Encoder value tests
    RUN_TEST(test_encoder_default_value);
    RUN_TEST(test_encoder_negative_value);
    RUN_TEST(test_encoder_large_positive_value);

    // Button ID tests
    RUN_TEST(test_button_id_valid_range);

    return UNITY_END();
}
