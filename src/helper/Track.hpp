#ifndef Track_h
#define Track_h

#include <Arduino.h>
#include <SD.h>

struct Track {
    String fileName;    // Name of the sample file (without .wav extension)
    uint32_t startPos;  // Start position in samples
    uint32_t endPos;    // End position in samples
    bool isAssigned;    // Whether this track has a sample assigned

    Track()
        : fileName(""),
          startPos(0),
          endPos(0),
          isAssigned(false) {}

    // Assign a sample to this track
    void assignSample(const String& fileName) {
        this->fileName = fileName;
        isAssigned = true;
    }

    // Clear this track
    void clear() {
        fileName = "";
        startPos = 0;
        endPos = 0;
        isAssigned = false;
    }

    // Get the full WAV file path
    String getWavPath() const { return "/RECORDINGS/" + fileName; }

    // Convert sample positions to byte positions for playback
    // WAV files have 44-byte header and 16-bit (2-byte) samples
    uint32_t getStartByte() const { return (startPos * 2) + 44; }

    uint32_t getEndByte() const {
        if (endPos == 0xFFFFFFFF) {
            return 0xFFFFFFFF;  // Play to end of file
        }
        return (endPos * 2) + 44;
    }
};

#endif
