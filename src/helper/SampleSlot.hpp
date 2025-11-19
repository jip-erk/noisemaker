#ifndef SampleSlot_h
#define SampleSlot_h

#include <Arduino.h>
#include <SD.h>

struct SampleSlot {
    String sampleName;       // Name of the sample file (without .wav extension)
    uint32_t startPos;       // Start position in samples
    uint32_t endPos;         // End position in samples
    uint8_t midiNote;        // MIDI note number (0-127)
    bool isAssigned;         // Whether this slot has a sample assigned

    SampleSlot() : sampleName(""), startPos(0), endPos(0), midiNote(0), isAssigned(false) {}

    // Load BDF file to get start and end positions
    bool loadBDF(const String& fileName) {
        String bdfPath = "/RECORDINGS/" + fileName + ".wav.bdf";

        if (!SD.exists(bdfPath.c_str())) {
            Serial.println("BDF file not found: " + bdfPath);
            // If no BDF file exists, use default full file playback
            startPos = 0;
            endPos = 0xFFFFFFFF;  // Max uint32_t indicates full file
            return false;
        }

        File bdfFile = SD.open(bdfPath.c_str(), FILE_READ);
        if (!bdfFile) {
            Serial.println("Failed to open BDF file: " + bdfPath);
            startPos = 0;
            endPos = 0xFFFFFFFF;
            return false;
        }

        // Read start position (4 bytes, little-endian)
        if (bdfFile.available() >= 8) {
            uint8_t bytes[8];
            bdfFile.read(bytes, 8);

            startPos = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
            endPos = bytes[4] | (bytes[5] << 8) | (bytes[6] << 16) | (bytes[7] << 24);

            Serial.println("Loaded BDF for " + fileName);
            Serial.print("  Start: ");
            Serial.print(startPos);
            Serial.print(", End: ");
            Serial.println(endPos);
        } else {
            Serial.println("BDF file too small: " + bdfPath);
            startPos = 0;
            endPos = 0xFFFFFFFF;
            bdfFile.close();
            return false;
        }

        bdfFile.close();
        return true;
    }

    // Assign a sample to this slot
    void assignSample(const String& fileName, uint8_t note) {
        sampleName = fileName;
        midiNote = note;
        loadBDF(fileName);
        isAssigned = true;
    }

    // Clear this slot
    void clear() {
        sampleName = "";
        startPos = 0;
        endPos = 0;
        midiNote = 0;
        isAssigned = false;
    }

    // Get the full WAV file path
    String getWavPath() const {
        return "/RECORDINGS/" + sampleName + ".wav";
    }

    // Convert sample positions to byte positions for playback
    // WAV files have 44-byte header and 16-bit (2-byte) samples
    uint32_t getStartByte() const {
        return (startPos * 2) + 44;
    }

    uint32_t getEndByte() const {
        if (endPos == 0xFFFFFFFF) {
            return 0xFFFFFFFF;  // Play to end of file
        }
        return (endPos * 2) + 44;
    }
};

#endif
