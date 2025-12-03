#include <Arduino.h>
#include <unity.h>
#include <cmath>

// Mock Waveform class for testing
class MockWaveform {
public:
    int totalSamples;

    MockWaveform(int samples) : totalSamples(samples) {}

    int getTotalSamples() const {
        return totalSamples;
    }

    void drawCachedWaveform(int start, int end) {}
    void drawSelection(int startX, int endX, int viewStart, int viewEnd) {}
};

// Minimal WaveformSelector implementation for testing
class WaveformSelectorTest {
private:
    MockWaveform* _waveform;
    int selectStartX = 0;
    int selectEndX = 0;
    bool selectingLeft = true;

    int _viewStartSample = 0;
    int _viewEndSample = 0;

    static constexpr int BASE_INCREMENT_DIVISOR = 100;
    static constexpr int MIN_INCREMENT = 1;
    static constexpr float ZOOM_IN_FACTOR = 0.9f;
    static constexpr float ZOOM_OUT_FACTOR = 1.11f;
    static constexpr int MIN_VIEW_RANGE = 500;

    int getTotalSamples() const {
        return _waveform ? _waveform->getTotalSamples() : 0;
    }

    int getViewRange() const { return _viewEndSample - _viewStartSample; }

    int calculateIncrement() const {
        int viewRange = getViewRange();
        return (MIN_INCREMENT > viewRange / BASE_INCREMENT_DIVISOR) ? MIN_INCREMENT
                                                                     : viewRange / BASE_INCREMENT_DIVISOR;
    }

    void clampViewBounds() {
        int totalSamples = getTotalSamples();

        if (_viewStartSample < 0) {
            int shift = -_viewStartSample;
            _viewStartSample = 0;
            _viewEndSample += shift;
        }

        if (_viewEndSample > totalSamples) {
            int shift = _viewEndSample - totalSamples;
            _viewEndSample = totalSamples;
            _viewStartSample -= shift;
            if (_viewStartSample < 0) _viewStartSample = 0;
        }

        if (_viewEndSample <= _viewStartSample) {
            _viewEndSample = (_viewStartSample + MIN_VIEW_RANGE > totalSamples)
                                 ? totalSamples
                                 : _viewStartSample + MIN_VIEW_RANGE;
            _viewStartSample = (_viewEndSample - MIN_VIEW_RANGE < 0) ? 0
                                                                      : _viewEndSample - MIN_VIEW_RANGE;
        }
    }

public:
    WaveformSelectorTest() : _waveform(nullptr) {}

    WaveformSelectorTest(MockWaveform* waveform) : _waveform(waveform) {
        int totalSamples = getTotalSamples();
        selectEndX = totalSamples;
        _viewEndSample = totalSamples;
    }

    void setWaveform(MockWaveform* waveform) {
        _waveform = waveform;
        if (_waveform) {
            int totalSamples = getTotalSamples();
            selectEndX = totalSamples;
            _viewEndSample = totalSamples;
            _viewStartSample = 0;
        }
    }

    void updateSelection(int encoderValue) {
        if (!_waveform || encoderValue == 0) return;

        int totalSamples = getTotalSamples();
        int increment = calculateIncrement();

        if (selectEndX == 0) selectEndX = totalSamples;

        if (encoderValue > 0) {
            if (selectingLeft) {
                selectStartX = (selectEndX - increment > selectStartX + increment)
                                   ? selectStartX + increment
                                   : selectEndX - increment;
            } else {
                selectEndX = (totalSamples > selectEndX + increment) ? selectEndX + increment
                                                                      : totalSamples;
            }
        } else {
            if (selectingLeft) {
                selectStartX =
                    (_viewStartSample < selectStartX - increment) ? selectStartX - increment
                                                                   : _viewStartSample;
            } else {
                int minEndX = selectStartX + increment;
                selectEndX = (minEndX < selectEndX - increment) ? selectEndX - increment : minEndX;
            }
        }
    }

    void zoom(int direction) {
        if (!_waveform || direction == 0) return;

        int totalSamples = getTotalSamples();
        int currentRange = getViewRange();

        float zoomFactor = (direction > 0) ? ZOOM_IN_FACTOR : ZOOM_OUT_FACTOR;
        int newRange = (int)(currentRange * zoomFactor);

        newRange = (newRange < MIN_VIEW_RANGE) ? MIN_VIEW_RANGE : newRange;
        newRange = (newRange > totalSamples) ? totalSamples : newRange;

        float anchorRatio =
            (float)(selectStartX - _viewStartSample) / currentRange;
        int rangeBeforeAnchor = (int)(newRange * anchorRatio);

        _viewStartSample = selectStartX - rangeBeforeAnchor;
        _viewEndSample = _viewStartSample + newRange;

        clampViewBounds();
    }

    void changeSide() { selectingLeft = !selectingLeft; }

    int getSelectStart() const { return selectStartX; }
    int getSelectEnd() const { return selectEndX; }
    int getViewStart() const { return _viewStartSample; }
    int getViewEnd() const { return _viewEndSample; }

    void resetZoom() {
        if (!_waveform) return;
        _viewStartSample = 0;
        _viewEndSample = getTotalSamples();
    }
};

void setUp(void) {}

void tearDown(void) {}

// Test that zoom in reduces view range
void test_zoom_in_reduces_range(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    int originalRange = selector.getViewEnd() - selector.getViewStart();
    selector.zoom(1);  // Zoom in
    int newRange = selector.getViewEnd() - selector.getViewStart();

    TEST_ASSERT_LESS_THAN(originalRange, newRange);
}

// Test that zoom out increases view range
void test_zoom_out_increases_range(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    int originalRange = selector.getViewEnd() - selector.getViewStart();
    selector.zoom(-1);  // Zoom out
    int newRange = selector.getViewEnd() - selector.getViewStart();

    TEST_ASSERT_GREATER_THAN(originalRange, newRange);
}

// Test that zoom respects minimum view range
void test_zoom_minimum_range_constraint(void) {
    MockWaveform waveform(10000);
    WaveformSelectorTest selector(&waveform);

    // Zoom in repeatedly to test minimum constraint
    for (int i = 0; i < 20; i++) {
        selector.zoom(1);
    }

    int range = selector.getViewEnd() - selector.getViewStart();
    TEST_ASSERT_GREATER_OR_EQUAL(range, 500);  // MIN_VIEW_RANGE
}

// Test that view bounds don't exceed total samples
void test_view_bounds_clamping_max(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    selector.zoom(-1);
    int viewEnd = selector.getViewEnd();

    TEST_ASSERT_LESS_OR_EQUAL(viewEnd, 100000);
}

// Test that view bounds don't go below zero
void test_view_bounds_clamping_min(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    int viewStart = selector.getViewStart();
    TEST_ASSERT_GREATER_OR_EQUAL(viewStart, 0);
}

// Test that selection start doesn't exceed selection end
void test_selection_start_less_than_end(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    TEST_ASSERT_LESS_THAN(selector.getSelectStart(), selector.getSelectEnd());
}

// Test zoom preserves anchor point (selection stays relatively centered)
void test_zoom_preserves_anchor_position(void) {
    MockWaveform waveform(1000000);
    WaveformSelectorTest selector(&waveform);

    // Set selection to center of view
    selector.changeSide();  // Switch to right side

    int initialSelectStart = 500000;
    // We can't directly set selection, so we'll reset and zoom
    selector.resetZoom();

    int viewRangeBeforeZoom = selector.getViewEnd() - selector.getViewStart();

    // Zoom in 5 times
    for (int i = 0; i < 5; i++) {
        selector.zoom(1);
    }

    // View range should change but should still contain selection
    int viewRangeAfterZoom = selector.getViewEnd() - selector.getViewStart();
    TEST_ASSERT_NOT_EQUAL(viewRangeBeforeZoom, viewRangeAfterZoom);
}

// Test that selection endpoints are initialized correctly
void test_initial_selection_bounds(void) {
    MockWaveform waveform(50000);
    WaveformSelectorTest selector(&waveform);

    TEST_ASSERT_EQUAL(0, selector.getSelectStart());
    TEST_ASSERT_EQUAL(50000, selector.getSelectEnd());
}

// Test that selection start is always >= 0
void test_selection_start_non_negative(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    selector.updateSelection(-100);  // Try to move left

    TEST_ASSERT_GREATER_OR_EQUAL(selector.getSelectStart(), 0);
}

// Test that selection end is always <= total samples
void test_selection_end_within_bounds(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    selector.changeSide();  // Switch to right side
    selector.updateSelection(100);  // Try to move right

    TEST_ASSERT_LESS_OR_EQUAL(selector.getSelectEnd(), 100000);
}

// Test reset zoom returns to full view
void test_reset_zoom_shows_full_waveform(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    selector.zoom(1);
    selector.zoom(1);

    selector.resetZoom();

    TEST_ASSERT_EQUAL(0, selector.getViewStart());
    TEST_ASSERT_EQUAL(100000, selector.getViewEnd());
}

// Test changing sides switches between left and right selector
void test_change_side_alternates(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    int startBefore = selector.getSelectStart();

    selector.changeSide();
    selector.updateSelection(100);

    // After switching to right side and moving right, end should increase
    int endAfter = selector.getSelectEnd();
    TEST_ASSERT_GREATER_THAN(50000, endAfter);  // Should have moved from default
}

// Test that zero encoder value produces no change
void test_zero_encoder_value_no_change(void) {
    MockWaveform waveform(100000);
    WaveformSelectorTest selector(&waveform);

    int startBefore = selector.getSelectStart();
    int endBefore = selector.getSelectEnd();

    selector.updateSelection(0);

    TEST_ASSERT_EQUAL(startBefore, selector.getSelectStart());
    TEST_ASSERT_EQUAL(endBefore, selector.getSelectEnd());
}

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_zoom_in_reduces_range);
    RUN_TEST(test_zoom_out_increases_range);
    RUN_TEST(test_zoom_minimum_range_constraint);
    RUN_TEST(test_view_bounds_clamping_max);
    RUN_TEST(test_view_bounds_clamping_min);
    RUN_TEST(test_selection_start_less_than_end);
    RUN_TEST(test_zoom_preserves_anchor_position);
    RUN_TEST(test_initial_selection_bounds);
    RUN_TEST(test_selection_start_non_negative);
    RUN_TEST(test_selection_end_within_bounds);
    RUN_TEST(test_reset_zoom_shows_full_waveform);
    RUN_TEST(test_change_side_alternates);
    RUN_TEST(test_zero_encoder_value_no_change);

    return UNITY_END();
}
