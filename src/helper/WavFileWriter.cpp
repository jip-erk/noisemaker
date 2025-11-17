#include "WavFileWriter.hpp"

#include <SPI.h>

WavFileWriter::WavFileWriter(AudioRecordQueue& queue)
    : m_isWriting(false), m_queue(queue), m_accumulatedSampleCount(0) {}

bool WavFileWriter::open(const char* fileName, unsigned int sampleRate,
                         unsigned int channelCount) {
    if (m_isWriting) {
        Serial.println("Cannot write WAV file. Already writing one.");
        return false;
    }

    if (SD.exists(fileName)) {
        SD.remove(fileName);
    }

    m_file = SD.open(fileName, FILE_WRITE);
    if (!m_file) {
        Serial.println("Could not open file while trying to write WAV file.");
        return false;
    }

    m_queue.begin();
    m_isWriting = true;
    m_totalBytesWritten = 0;

    writeHeader(sampleRate, channelCount);
    return true;
}

bool WavFileWriter::isWriting() { return m_isWriting; }

void WavFileWriter::writeHeader(unsigned int sampleRate,
                                unsigned int channelCount) {
    // Write the main chunk ID
    uint8_t mainChunkId[4] = {'R', 'I', 'F', 'F'};
    m_file.write(mainChunkId, sizeof(mainChunkId));

    // Write the main chunk header
    uint32_t mainChunkSize = 0;  // placeholder, will be written on closing
    encode(m_file, mainChunkSize);
    uint8_t mainChunkFormat[4] = {'W', 'A', 'V', 'E'};
    m_file.write(mainChunkFormat, sizeof(mainChunkFormat));

    // Write the sub-chunk 1 ("format") id and size
    uint8_t fmtChunkId[4] = {'f', 'm', 't', ' '};
    m_file.write(fmtChunkId, sizeof(fmtChunkId));
    uint32_t fmtChunkSize = 16;
    encode(m_file, fmtChunkSize);

    // Write the format (PCM)
    uint16_t format = 1;
    encode(m_file, format);

    // Write the sound attributes
    encode(m_file, static_cast<uint16_t>(channelCount));
    encode(m_file, static_cast<uint32_t>(sampleRate));
    uint32_t byteRate = sampleRate * channelCount * 2;
    encode(m_file, byteRate);
    uint16_t blockAlign = channelCount * 2;
    encode(m_file, blockAlign);
    uint16_t bitsPerSample = 16;
    encode(m_file, bitsPerSample);

    // Write the sub-chunk 2 ("data") id and size
    uint8_t dataChunkId[4] = {'d', 'a', 't', 'a'};
    m_file.write(dataChunkId, sizeof(dataChunkId));
    uint32_t dataChunkSize = 0;  // placeholder, will be written on closing
    encode(m_file, dataChunkSize);

    m_totalBytesWritten += 44;
}

bool WavFileWriter::update() {
    if (!m_isWriting) return false;

    if (m_queue.available() < 2) return false;

    // Fetch 2 blocks from the audio library and copy
    // into a 512 byte buffer.  The Arduino SD library
    // is most efficient when full 512 byte sector size
    // writes are used.
    memcpy(m_buffer, m_queue.readBuffer(), 256);
    m_queue.freeBuffer();
    memcpy(m_buffer + 256, m_queue.readBuffer(), 256);
    m_queue.freeBuffer();

    // write all 512 bytes to the SD card
    m_file.write(m_buffer, 512);
    m_totalBytesWritten += 512;

    for (size_t i = 0; i < 512; i += 2) {
        if (m_accumulatedSampleCount < MAX_ACCUMULATED_SAMPLES) {
            // Convert little-endian bytes to int16_t
            int16_t sample = (m_buffer[i + 1] << 8) | m_buffer[i];
            m_accumulatedBuffer[m_accumulatedSampleCount++] = sample;
        }
    }

    return true;
}

bool WavFileWriter::close() {
    if (!m_isWriting) return false;

    m_queue.end();
    while (m_queue.available() > 0) {
        m_file.write(reinterpret_cast<const uint8_t*>(m_queue.readBuffer()),
                     256);
        m_queue.freeBuffer();
        m_totalBytesWritten += 256;
    }

    Serial.print("Done! Max no. of audio blocks used: ");
    Serial.println(AudioMemoryUsageMax());
    Serial.print("Bytes written: ");
    Serial.println(m_totalBytesWritten);

    m_file.flush();

    // Update the main chunk size and data sub-chunk size
    uint32_t mainChunkSize = m_totalBytesWritten - 8;  // 8 bytes RIFF header
    uint32_t dataChunkSize =
        m_totalBytesWritten - 44;  // 44 bytes RIFF + WAVE headers
    m_file.seek(4);
    encode(m_file, mainChunkSize);
    m_file.seek(40);
    encode(m_file, dataChunkSize);

    m_file.close();

    m_isWriting = false;
    return true;
}

const int16_t* WavFileWriter::getAccumulatedBuffer(size_t& sampleCount) {
    sampleCount = m_accumulatedSampleCount;
    return m_accumulatedBuffer;
}

void WavFileWriter::clearAccumulatedBuffer() { m_accumulatedSampleCount = 0; }

void WavFileWriter::encode(File& file, uint16_t value) {
    uint8_t bytes[] = {static_cast<uint8_t>(value & 0xFF),
                       static_cast<uint8_t>(value >> 8)};
    file.write(reinterpret_cast<const uint8_t*>(bytes), sizeof(bytes));
}

void WavFileWriter::encode(File& file, uint32_t value) {
    uint8_t bytes[] = {
        static_cast<uint8_t>(value & 0x000000FF),
        static_cast<uint8_t>((value & 0x0000FF00) >> 8),
        static_cast<uint8_t>((value & 0x00FF0000) >> 16),
        static_cast<uint8_t>((value & 0xFF000000) >> 24),
    };
    file.write(reinterpret_cast<const uint8_t*>(bytes), sizeof(bytes));
}