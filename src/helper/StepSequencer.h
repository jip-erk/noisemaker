#ifndef StepSequencer_h
#define StepSequencer_h

#include <Arduino.h>

class StepSequencer {
   public:
    static const int NUM_TRACKS = 4;
    static const int NUM_STEPS = 16;

    StepSequencer();

    // Playback control
    void start();
    void stop();
    bool isPlaying() const;
    void setBPM(int bpm);
    int getBPM() const;

    // Step management
    void toggleStep(int track, int step);
    bool getStep(int track, int step) const;
    void clearAllSteps();

    // Timing and Progression
    // Advance step and return current step index
    int advance();
    int getCurrentStep() const;
    long calculateStepIntervalMicros() const;

    // Call this inside the loop to check if tracks are active at current step
    bool isTrackActive(int track) const;

    // Access needed for UI
    const bool (*getGrid() const)[NUM_STEPS] { return _grid; }

   private:
    bool _grid[NUM_TRACKS][NUM_STEPS];
    int _currentStep;
    int _bpm;
    bool _playing;
};

#endif
