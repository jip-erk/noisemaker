#ifndef NAME_GENERATOR_HPP
#define NAME_GENERATOR_HPP

#include <Arduino.h>

class NameGenerator {
   private:
    static const uint8_t ADJ_COUNT = 50;
    static const uint8_t NOUN_COUNT = 50;

    // Helper method to get adjective by index
    String getAdjective(uint8_t idx) {
        static const char adjectives[50][10] PROGMEM = {
            "Bright", "Dark",   "Swift", "Deep",  "Wild",  "Cool",   "Warm",
            "Bold",   "Calm",   "Fast",  "Slow",  "High",  "Low",    "Grand",
            "Tiny",   "Pure",   "Rare",  "True",  "Fair",  "Fine",   "Sharp",
            "Soft",   "Hard",   "Clear", "Loud",  "Quiet", "Smooth", "Rough",
            "Gentle", "Fierce", "Brave", "Noble", "Keen",  "Wise",   "Quick",
            "Light",  "Heavy",  "Fresh", "Crisp", "Dense", "Thin",   "Thick",
            "Wide",   "Narrow", "Tall",  "Short", "Long",  "Stark",  "Vivid",
            "Pale"};
        char buffer[10];
        strcpy_P(buffer, (PGM_P)adjectives[idx]);
        return String(buffer);
    }

    // Helper method to get noun by index
    String getNoun(uint8_t idx) {
        static const char nouns[50][10] PROGMEM = {
            "Wave",  "Storm",   "Wind",  "Fire",   "Water", "Earth",  "Stone",
            "Iron",  "Steel",   "Cloud", "Sky",    "Sun",   "Moon",   "Star",
            "Light", "Shadow",  "Peak",  "Valley", "River", "Lake",   "Ocean",
            "Sea",   "Forest",  "Tree",  "Leaf",   "Root",  "Branch", "Bird",
            "Wolf",  "Bear",    "Eagle", "Hawk",   "Raven", "Fox",    "Lion",
            "Tiger", "Dragon",  "Flame", "Blaze",  "Spark", "Frost",  "Ice",
            "Snow",  "Thunder", "Rain",  "Mist",   "Dawn",  "Dusk",   "Night",
            "Day"};
        char buffer[10];
        strcpy_P(buffer, (PGM_P)nouns[idx]);
        return String(buffer);
    }

   public:
    NameGenerator() { randomSeed(analogRead(0)); }

    // Generate a random name combining adjective + noun
    String generate() {
        uint8_t adjIdx = random(ADJ_COUNT);
        uint8_t nounIdx = random(NOUN_COUNT);

        String result = "";
        result += getAdjective(adjIdx);
        result += getNoun(nounIdx);

        return result;
    }

    // Generate with custom separator
    String generate(const char* separator) {
        uint8_t adjIdx = random(ADJ_COUNT);
        uint8_t nounIdx = random(NOUN_COUNT);

        String result = "";
        result += getAdjective(adjIdx);
        result += separator;
        result += getNoun(nounIdx);

        return result;
    }

    // Generate a complete audio filename with extension
    String generateAudioFilename() { return generate(); }

    // Get total possible combinations
    unsigned int getTotalCombinations() const { return ADJ_COUNT * NOUN_COUNT; }
};

#endif  // NAME_GENERATOR_HPP