#ifndef WAVFILEWRITER_HPP
#define WAVFILEWRITER_HPP

#include <Arduino.h>
#include <Audio.h>
#include <SD.h>

class WavFileWriter {
   public:
    WavFileWriter(AudioRecordQueue& queue);

    const int16_t* getAccumulatedBuffer(size_t& sampleCount);
    void clearAccumulatedBuffer();

    bool open(const char* fileName, unsigned int sampleRate = 44100,
              unsigned int channelCount = 1);
    bool isWriting();
    bool update();
    bool close();
    File& getFile() { return m_file; }

   private:
    void writeHeader(unsigned int sampleRate, unsigned int channelCount);
    void encode(File& file, uint16_t value);
    void encode(File& file, uint32_t value);

    bool m_isWriting;
    AudioRecordQueue& m_queue;
    File m_file;
    uint8_t m_buffer[512];
    uint32_t m_totalBytesWritten;

    static const size_t MAX_ACCUMULATED_SAMPLES = 44100;  // ~0.5 sec at 44.1kHz
    int16_t m_accumulatedBuffer[MAX_ACCUMULATED_SAMPLES];
    size_t m_accumulatedSampleCount;
};

#endif  // WAVFILEWRITER_HPP
