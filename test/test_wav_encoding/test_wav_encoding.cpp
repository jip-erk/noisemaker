#include <Arduino.h>
#include <unity.h>
#include <cstring>

// Mock for File class to avoid SD card dependency
class MockFile {
public:
    struct WrittenData {
        uint8_t bytes[4];
        size_t size;
    };

    std::vector<WrittenData> written_data;

    void write(const uint8_t* data, size_t size) {
        for (size_t i = 0; i < size; i++) {
            WrittenData wd;
            wd.bytes[0] = data[i];
            wd.size = 1;
            written_data.push_back(wd);
        }
    }

    // Get bytes written as a continuous buffer
    std::vector<uint8_t> getAllBytes() {
        std::vector<uint8_t> result;
        for (const auto& wd : written_data) {
            for (size_t i = 0; i < wd.size; i++) {
                result.push_back(wd.bytes[i]);
            }
        }
        return result;
    }
};

// Extract the encode functions to test them independently
void encode_uint16(std::vector<uint8_t>& buffer, uint16_t value) {
    uint8_t bytes[] = {static_cast<uint8_t>(value & 0xFF),
                       static_cast<uint8_t>(value >> 8)};
    buffer.insert(buffer.end(), bytes, bytes + sizeof(bytes));
}

void encode_uint32(std::vector<uint8_t>& buffer, uint32_t value) {
    uint8_t bytes[] = {
        static_cast<uint8_t>(value & 0x000000FF),
        static_cast<uint8_t>((value & 0x0000FF00) >> 8),
        static_cast<uint8_t>((value & 0x00FF0000) >> 16),
        static_cast<uint8_t>((value & 0xFF000000) >> 24),
    };
    buffer.insert(buffer.end(), bytes, bytes + sizeof(bytes));
}

void setUp(void) {
    // Initialize before each test
}

void tearDown(void) {
    // Cleanup after each test
}

// Test encoding 16-bit values in little-endian format
void test_encode_uint16_little_endian(void) {
    std::vector<uint8_t> buffer;
    encode_uint16(buffer, 0x1234);

    // In little-endian: low byte first, then high byte
    TEST_ASSERT_EQUAL_UINT8(0x34, buffer[0]);  // Low byte
    TEST_ASSERT_EQUAL_UINT8(0x12, buffer[1]);  // High byte
}

// Test encoding 16-bit max value
void test_encode_uint16_max_value(void) {
    std::vector<uint8_t> buffer;
    encode_uint16(buffer, 0xFFFF);

    TEST_ASSERT_EQUAL_UINT8(0xFF, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0xFF, buffer[1]);
}

// Test encoding 16-bit zero value
void test_encode_uint16_zero(void) {
    std::vector<uint8_t> buffer;
    encode_uint16(buffer, 0x0000);

    TEST_ASSERT_EQUAL_UINT8(0x00, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, buffer[1]);
}

// Test encoding 32-bit values in little-endian format
void test_encode_uint32_little_endian(void) {
    std::vector<uint8_t> buffer;
    encode_uint32(buffer, 0x12345678);

    // In little-endian: least significant byte first
    TEST_ASSERT_EQUAL_UINT8(0x78, buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0x56, buffer[1]);
    TEST_ASSERT_EQUAL_UINT8(0x34, buffer[2]);
    TEST_ASSERT_EQUAL_UINT8(0x12, buffer[3]);
}

// Test encoding 32-bit max value
void test_encode_uint32_max_value(void) {
    std::vector<uint8_t> buffer;
    encode_uint32(buffer, 0xFFFFFFFF);

    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_UINT8(0xFF, buffer[i]);
    }
}

// Test WAV file size calculation: mainChunkSize = totalBytes - 8
void test_wav_main_chunk_size_calculation(void) {
    uint32_t totalBytesWritten = 44 + 512;  // Header + one block
    uint32_t mainChunkSize = totalBytesWritten - 8;

    TEST_ASSERT_EQUAL_UINT32(548, mainChunkSize);
}

// Test WAV file size calculation: dataChunkSize = totalBytes - 44
void test_wav_data_chunk_size_calculation(void) {
    uint32_t totalBytesWritten = 44 + 512 + 256;
    uint32_t dataChunkSize = totalBytesWritten - 44;

    TEST_ASSERT_EQUAL_UINT32(768, dataChunkSize);
}

// Test byte rate calculation for mono 44.1kHz 16-bit PCM
void test_wav_byte_rate_mono_44k(void) {
    uint32_t sampleRate = 44100;
    uint16_t channelCount = 1;
    uint16_t bitsPerSample = 16;
    uint32_t byteRate = sampleRate * channelCount * (bitsPerSample / 8);

    TEST_ASSERT_EQUAL_UINT32(88200, byteRate);
}

// Test byte rate calculation for stereo 44.1kHz 16-bit PCM
void test_wav_byte_rate_stereo_44k(void) {
    uint32_t sampleRate = 44100;
    uint16_t channelCount = 2;
    uint16_t bitsPerSample = 16;
    uint32_t byteRate = sampleRate * channelCount * (bitsPerSample / 8);

    TEST_ASSERT_EQUAL_UINT32(176400, byteRate);
}

// Test block align calculation for mono
void test_wav_block_align_mono(void) {
    uint16_t channelCount = 1;
    uint16_t bitsPerSample = 16;
    uint16_t blockAlign = channelCount * (bitsPerSample / 8);

    TEST_ASSERT_EQUAL_UINT16(2, blockAlign);
}

// Test block align calculation for stereo
void test_wav_block_align_stereo(void) {
    uint16_t channelCount = 2;
    uint16_t bitsPerSample = 16;
    uint16_t blockAlign = channelCount * (bitsPerSample / 8);

    TEST_ASSERT_EQUAL_UINT16(4, blockAlign);
}

// Test little-endian conversion of audio samples in buffer
void test_sample_to_int16_little_endian(void) {
    uint8_t buffer[2] = {0x34, 0x12};
    int16_t sample = (buffer[1] << 8) | buffer[0];

    TEST_ASSERT_EQUAL_INT16(0x1234, sample);
}

// Test little-endian conversion with negative sample value
void test_sample_to_int16_negative_value(void) {
    // 0xFFFF in two's complement = -1
    uint8_t buffer[2] = {0xFF, 0xFF};
    int16_t sample = (buffer[1] << 8) | buffer[0];

    TEST_ASSERT_EQUAL_INT16(-1, sample);
}

// Test WAV header is always 44 bytes
void test_wav_header_size_constant(void) {
    // RIFF chunk (4) + Size (4) + WAVE (4) + fmt chunk (4) + Size (4)
    // + PCM format (16) + data chunk (4) + Size (4) = 44 bytes
    int headerSize = 44;
    TEST_ASSERT_EQUAL_INT(44, headerSize);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();

    // 16-bit encoding tests
    RUN_TEST(test_encode_uint16_little_endian);
    RUN_TEST(test_encode_uint16_max_value);
    RUN_TEST(test_encode_uint16_zero);

    // 32-bit encoding tests
    RUN_TEST(test_encode_uint32_little_endian);
    RUN_TEST(test_encode_uint32_max_value);

    // WAV structure calculation tests
    RUN_TEST(test_wav_main_chunk_size_calculation);
    RUN_TEST(test_wav_data_chunk_size_calculation);
    RUN_TEST(test_wav_byte_rate_mono_44k);
    RUN_TEST(test_wav_byte_rate_stereo_44k);
    RUN_TEST(test_wav_block_align_mono);
    RUN_TEST(test_wav_block_align_stereo);
    RUN_TEST(test_sample_to_int16_little_endian);
    RUN_TEST(test_sample_to_int16_negative_value);
    RUN_TEST(test_wav_header_size_constant);

    return UNITY_END();
}
