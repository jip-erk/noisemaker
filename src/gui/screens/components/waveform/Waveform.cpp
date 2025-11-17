#include "Waveform.h"

Waveform::Waveform() : _screen(nullptr) { clear(); }

Waveform::Waveform(Screen* screen) : _screen(screen) { clear(); }

Waveform::Waveform(Screen* screen, int x, int y, int width, int height)
    : _screen(screen), _x(x), _y(y), _width(width), _height(height) {
    clear();
}

Waveform::~Waveform() { freeCacheMemory(); }

void Waveform::freeCacheMemory() {
    if (_minCache) {
        delete[] _minCache;
        _minCache = nullptr;
    }
    if (_maxCache) {
        delete[] _maxCache;
        _maxCache = nullptr;
    }
    _cacheSize = 0;
}

void Waveform::drawWaveformFrame() {
    if (!_screen) return;

    auto* display = _screen->getDisplay();
    int centerY = _y + (_height / 2);

    // Clear the area (fill with background color)
    display->setDrawColor(0);
    display->drawBox(_x, _y, _width, _height);
    display->setDrawColor(1);

    // Draw rounded frame border
    display->drawRFrame(_x, _y, _width, _height, 3);

    // Draw center line
    display->drawHLine(_x + 1, centerY, _width - 2);
}

void Waveform::drawWaveformBar(int x, int16_t minSample, int16_t maxSample,
                                float amplificationGain) {
    if (!_screen) return;

    auto* display = _screen->getDisplay();

    // Apply amplification
    int32_t amplifiedMin = (int32_t)(minSample * amplificationGain);
    int32_t amplifiedMax = (int32_t)(maxSample * amplificationGain);

    // Clamp to int16_t range to prevent overflow
    amplifiedMin = constrain(amplifiedMin, -32768, 32767);
    amplifiedMax = constrain(amplifiedMax, -32768, 32767);

    // Map to screen coordinates
    int yMin = map(amplifiedMax, -32768, 32767, _y + _height - 2, _y + 1);
    int yMax = map(amplifiedMin, -32768, 32767, _y + _height - 2, _y + 1);

    // Draw vertical line representing the waveform envelope
    display->drawLine(_x + 1 + x, yMin, _x + 1 + x, yMax);
}

bool Waveform::loadWaveformFile(const char* fileName, int maxMemoryKB) {
    // Free existing cache
    freeCacheMemory();

    // Open file
    File wavFile = SD.open(fileName);
    if (!wavFile) {
        Serial.println("Failed to open WAV file");
        return false;
    }

    const int WAV_HEADER_SIZE = 44;
    _totalSamples = (wavFile.size() - WAV_HEADER_SIZE) / sizeof(int16_t);

    if (_totalSamples == 0) {
        wavFile.close();
        return false;
    }

    // Calculate optimal downsampling ratio based on memory limit
    // Each cache point needs 2 bytes (min) + 2 bytes (max) = 4 bytes
    int maxCachePoints = (maxMemoryKB * 1024) / 4;

    // Determine cache size and downsampling ratio
    if (_totalSamples <= maxCachePoints) {
        // Can store all samples
        _cacheSize = _totalSamples;
        _samplesPerCachePoint = 1;
    } else {
        // Need to downsample
        _cacheSize = maxCachePoints;
        _samplesPerCachePoint =
            (_totalSamples + maxCachePoints - 1) / maxCachePoints;
    }

    // Allocate cache memory
    _minCache = new int16_t[_cacheSize];
    _maxCache = new int16_t[_cacheSize];

    if (!_minCache || !_maxCache) {
        Serial.println("Failed to allocate cache memory");
        freeCacheMemory();
        wavFile.close();
        return false;
    }

    Serial.printf(
        "Caching waveform: %d samples -> %d cache points (ratio: %d:1)\n",
        _totalSamples, _cacheSize, _samplesPerCachePoint);

    // Read and downsample the entire file
    const int READ_BUFFER_SIZE = 512;  // Read in chunks for efficiency
    int16_t readBuffer[READ_BUFFER_SIZE];

    wavFile.seek(WAV_HEADER_SIZE);

    for (int cacheIdx = 0; cacheIdx < _cacheSize; cacheIdx++) {
        int sampleStart = cacheIdx * _samplesPerCachePoint;
        int sampleEnd = min(sampleStart + _samplesPerCachePoint, _totalSamples);
        int samplesToRead = sampleEnd - sampleStart;

        int16_t minVal = 32767;
        int16_t maxVal = -32768;

        // Read samples in chunks
        int samplesRead = 0;
        while (samplesRead < samplesToRead) {
            int chunkSize = min(READ_BUFFER_SIZE, samplesToRead - samplesRead);
            int bytesRead =
                wavFile.read((uint8_t*)readBuffer, chunkSize * sizeof(int16_t));
            int actualSamples = bytesRead / sizeof(int16_t);

            // Find min/max in this chunk
            for (int i = 0; i < actualSamples; i++) {
                int16_t sample = readBuffer[i];
                if (sample < minVal) minVal = sample;
                if (sample > maxVal) maxVal = sample;
            }

            samplesRead += actualSamples;
        }

        _minCache[cacheIdx] = minVal;
        _maxCache[cacheIdx] = maxVal;
    }

    wavFile.close();

    Serial.println("Waveform cached successfully");

    return true;
}
void Waveform::drawCachedWaveform(int startSample, int endSample) {
    if (!_screen || !_minCache || !_maxCache) return;

    // Clamp to valid range
    if (startSample < 0) startSample = 0;
    if (endSample == 0) endSample = _totalSamples;
    if (endSample > _totalSamples) endSample = _totalSamples;
    if (endSample <= startSample) endSample = startSample + 1;

    int displayWidth = _width - 2;

    // Draw frame, border, and center line
    drawWaveformFrame();

    // Convert sample positions to cache indices
    int startCacheIdx = startSample / _samplesPerCachePoint;
    int endCacheIdx =
        (endSample + _samplesPerCachePoint - 1) / _samplesPerCachePoint;

    if (endCacheIdx > _cacheSize) endCacheIdx = _cacheSize;

    int cacheSamplesInView = endCacheIdx - startCacheIdx;
    if (cacheSamplesInView == 0) cacheSamplesInView = 1;

    // === AMPLIFICATION SETTINGS ===
    // Adjust this gain factor to increase sensitivity (2.0 = 2x, 4.0 = 4x,
    // etc.)
    float amplificationGain = 3.0;  // Start with 3x amplification

    // Optional: Auto-gain - analyze the view to find peak and normalize
    bool useAutoGain = false;  // Set to true for automatic gain adjustment

    if (useAutoGain) {
        // Find the maximum absolute value in the visible range
        int16_t globalMax = 0;
        for (int i = startCacheIdx; i < endCacheIdx; i++) {
            int16_t absMin = abs(_minCache[i]);
            int16_t absMax = abs(_maxCache[i]);
            int16_t localMax = max(absMin, absMax);
            if (localMax > globalMax) globalMax = localMax;
        }

        // Calculate auto-gain to use ~80% of available height
        if (globalMax > 100) {  // Avoid division by very small numbers
            amplificationGain = (32767.0 * 0.8) / globalMax;
            // Limit maximum gain to prevent excessive amplification of noise
            if (amplificationGain > 10.0) amplificationGain = 10.0;
        }
    }

    // Draw waveform from cache
    for (int x = 0; x < displayWidth; x++) {
        int cacheStart =
            startCacheIdx + (x * cacheSamplesInView / displayWidth);
        int cacheEnd =
            startCacheIdx + ((x + 1) * cacheSamplesInView / displayWidth);

        if (cacheEnd > endCacheIdx) cacheEnd = endCacheIdx;
        if (cacheStart >= cacheEnd) cacheEnd = cacheStart + 1;

        int16_t minSample = 32767;
        int16_t maxSample = -32768;

        // Find min/max from cached values
        for (int i = cacheStart; i < cacheEnd; i++) {
            if (_minCache[i] < minSample) minSample = _minCache[i];
            if (_maxCache[i] > maxSample) maxSample = _maxCache[i];
        }

        // Draw the waveform bar using shared helper
        drawWaveformBar(x, minSample, maxSample, amplificationGain);
    }
}

void Waveform::setPosition(int x, int y) {
    _x = x;
    _y = y;
}

void Waveform::setSize(int width, int height) {
    _width = width;
    _height = height;
}

void Waveform::clear() {
    for (int i = 0; i < MAX_WAVEFORM_POINTS; i++) {
        _liveMinData[i] = 0;
        _liveMaxData[i] = 0;
    }
    _writeIndex = 0;
}

void Waveform::addAudioData(const int16_t* audioBuffer, int bufferSize) {
    if (!audioBuffer || bufferSize == 0) return;

    int displayWidth = _width - 2;  // Account for border

    // Only add data if we haven't filled the display yet
    if (_writeIndex >= displayWidth) return;

    // Find min and max values from the entire buffer
    int16_t minVal = 32767;
    int16_t maxVal = -32768;

    for (int i = 0; i < bufferSize; i++) {
        int16_t sample = audioBuffer[i];
        if (sample < minVal) minVal = sample;
        if (sample > maxVal) maxVal = sample;
    }

    // Store min/max values at current write position
    _liveMinData[_writeIndex] = minVal;
    _liveMaxData[_writeIndex] = maxVal;
    _writeIndex++;
}

void Waveform::drawSelection(int selectStart, int selectEnd, int startSample,
                             int endSample) {
    if (!_screen) return;

    auto* display = _screen->getDisplay();
    int displayWidth = _width - 2;
    int viewSamples = endSample - startSample;

    // Check if selection is visible in current view
    if (selectEnd < startSample || selectStart > endSample) {
        return;  // Selection is completely outside visible range
    }

    // Clamp selection to visible sample range
    int visibleSelectStart = std::max(selectStart, startSample);
    int visibleSelectEnd = std::min(selectEnd, endSample);

    // Convert sample positions to pixel positions
    int selectStartX =
        _x + 1 +
        ((visibleSelectStart - startSample) * displayWidth / viewSamples);
    int selectEndX =
        _x + 1 +
        ((visibleSelectEnd - startSample) * displayWidth / viewSamples);

    // Additional pixel clamping for safety
    if (selectStartX < _x + 1) selectStartX = _x + 1;
    if (selectEndX > _x + _width - 1) selectEndX = _x + _width - 1;

    // Only draw if there's a visible width
    int selectionWidth = selectEndX - selectStartX;
    if (selectionWidth > 0) {
        display->setDrawColor(2);  // XOR mode
        display->drawBox(selectStartX, _y + 2, selectionWidth, _height - 4);
        display->setDrawColor(1);  // Back to normal
    }
}

void Waveform::drawWaveform() {
    if (!_screen) return;

    int displayWidth = _width - 2;

    // Draw frame, border, and center line using shared helper
    drawWaveformFrame();

    // Amplification gain for live recording (same as cached waveform)
    float amplificationGain = 3.0;

    // Draw waveform data using the same rendering logic as cached waveform
    for (int i = 0; i < displayWidth && i < MAX_WAVEFORM_POINTS; i++) {
        int16_t minSample = _liveMinData[i];
        int16_t maxSample = _liveMaxData[i];

        // Draw the waveform bar using shared helper
        drawWaveformBar(i, minSample, maxSample, amplificationGain);
    }

    // Draw current write index indicator line
    if (_writeIndex < displayWidth) {
        auto* display = _screen->getDisplay();
        int indicatorX = _x + 1 + _writeIndex;
        display->drawVLine(indicatorX, _y + 1, _height - 2);
    }
}