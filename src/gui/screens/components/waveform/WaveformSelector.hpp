#ifndef WaveformSelector_h
#define WaveformSelector_h

#include "Waveform.h"

class WaveformSelector {
   private:
    Waveform* _waveform;
    int selectStartX = 0;
    int selectEndX = 0;
    bool selectingLeft = true;

    int _viewStartSample = 0;
    int _viewEndSample = 0;

    // Sensitivity constants
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
        // Automatically scales with zoom level - more zoomed = finer control
        int viewRange = getViewRange();
        return std::max(MIN_INCREMENT, viewRange / BASE_INCREMENT_DIVISOR);
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
            _viewEndSample =
                std::min(_viewStartSample + MIN_VIEW_RANGE, totalSamples);
            _viewStartSample = std::max(0, _viewEndSample - MIN_VIEW_RANGE);
        }
    }

   public:
    WaveformSelector() : _waveform(nullptr) {}

    WaveformSelector(Waveform* waveform) : _waveform(waveform) {
        int totalSamples = getTotalSamples();
        selectEndX = totalSamples;
        _viewEndSample = totalSamples;
    }

    void updateSelection(int encoderValue) {
        if (!_waveform || encoderValue == 0) return;

        int totalSamples = getTotalSamples();
        int increment = calculateIncrement();

        if (selectEndX == 0) selectEndX = totalSamples;

        if (encoderValue > 0) {
            if (selectingLeft) {
                selectStartX =
                    std::min(selectEndX - increment, selectStartX + increment);
            } else {
                selectEndX = std::min(totalSamples, selectEndX + increment);
            }
        } else {
            if (selectingLeft) {
                selectStartX =
                    std::max(_viewStartSample, selectStartX - increment);
            } else {
                selectEndX =
                    std::max(selectStartX + increment, selectEndX - increment);
            }
        }
    }

    void zoom(int direction) {
        if (!_waveform || direction == 0) return;

        int totalSamples = getTotalSamples();
        int currentRange = getViewRange();

        float zoomFactor = (direction > 0) ? ZOOM_IN_FACTOR : ZOOM_OUT_FACTOR;
        int newRange = (int)(currentRange * zoomFactor);

        newRange = std::max(MIN_VIEW_RANGE, std::min(newRange, totalSamples));

        float anchorRatio =
            (float)(selectStartX - _viewStartSample) / currentRange;
        int rangeBeforeAnchor = (int)(newRange * anchorRatio);

        _viewStartSample = selectStartX - rangeBeforeAnchor;
        _viewEndSample = _viewStartSample + newRange;

        clampViewBounds();
    }

    void changeSide() { selectingLeft = !selectingLeft; }

    void draw() {
        if (!_waveform) return;

        _waveform->drawCachedWaveform(_viewStartSample, _viewEndSample);

        if (selectStartX >= 0 && selectEndX > selectStartX) {
            _waveform->drawSelection(selectStartX, selectEndX, _viewStartSample,
                                     _viewEndSample);
        }
    }

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

#endif