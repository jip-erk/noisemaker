#include "StepSequencer.h"

StepSequencer::StepSequencer() {
    _playing = false;
    _currentStep = 0;
    _bpm = 120;
    clearAllSteps();
}

void StepSequencer::start() {
    _playing = true;
    _currentStep = -1;  // Next advance will go to 0
}

void StepSequencer::stop() { _playing = false; }

bool StepSequencer::isPlaying() const { return _playing; }

void StepSequencer::setBPM(int bpm) {
    _bpm = constrain(
        bpm, 60,
        180);  // Keeping limits from original code concept, though explicit
               // limits weren't seen, 60-180 comments were present
}

int StepSequencer::getBPM() const { return _bpm; }

void StepSequencer::toggleStep(int track, int step) {
    if (track >= 0 && track < NUM_TRACKS && step >= 0 && step < NUM_STEPS) {
        _grid[track][step] = !_grid[track][step];
    }
}

bool StepSequencer::getStep(int track, int step) const {
    if (track >= 0 && track < NUM_TRACKS && step >= 0 && step < NUM_STEPS) {
        return _grid[track][step];
    }
    return false;
}

void StepSequencer::clearAllSteps() {
    for (int t = 0; t < NUM_TRACKS; t++) {
        for (int s = 0; s < NUM_STEPS; s++) {
            _grid[t][s] = false;
        }
    }
}

int StepSequencer::advance() {
    _currentStep = (_currentStep + 1) % NUM_STEPS;
    return _currentStep;
}

int StepSequencer::getCurrentStep() const { return _currentStep; }

long StepSequencer::calculateStepIntervalMicros() const {
    // 16th note = (60 / BPM / 4) seconds = (60000000 / BPM / 4) microseconds
    if (_bpm <= 0) return 1000000;
    return (60000000 / _bpm / 4);
}

bool StepSequencer::isTrackActive(int track) const {
    if (track < 0 || track >= NUM_TRACKS) return false;
    // If stopped, no tracks are active? Or checking purely grid logic?
    // Let's assume grid logic at current step.
    if (_currentStep < 0 || _currentStep >= NUM_STEPS) return false;
    return _grid[track][_currentStep];
}
