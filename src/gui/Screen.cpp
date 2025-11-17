#include "Screen.h"

Screen::Screen() : u8g2(U8G2_R0, U8X8_PIN_NONE) {}

void Screen::begin() {
    u8g2.begin();
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_tiny5_tr);
}

void Screen::clear() { u8g2.clearBuffer(); }

void Screen::display() { u8g2.sendBuffer(); }

void Screen::drawStr(int x, int y, const char* str) { u8g2.drawStr(x, y, str); }

// u8g2_font_pixzillav1_tr, u8g2_font_doomalpha04_tr
void Screen::setHeaderFont() { u8g2.setFont(u8g2_font_doomalpha04_tr); }

void Screen::setNormalFont() { u8g2.setFont(u8g2_font_tiny5_tr); }

int Screen::getWidth() { return 128; }

void Screen::drawBox(int x, int y, int w, int h) {
    // Set draw color to white/foreground
    u8g2.setDrawColor(1);
    u8g2.drawRBox(x, y, w, h, 3);
}

void Screen::drawItemList(int x, int y, const char* items[],
                          int selectedIndex) {
    const int lineHeight = 10;      // Height for each line including spacing
    const int maxVisibleItems = 6;  // Maximum items that fit on screen

    u8g2.clearBuffer();

    // Calculate start index to keep selected item visible
    int startIdx =
        max(0, min(selectedIndex - maxVisibleItems + 1, selectedIndex));

    for (int i = 0; i < maxVisibleItems && items[i + startIdx] != nullptr;
         i++) {
        int currentY = y + (i * lineHeight);

        // Draw selection indicator for selected item
        if (i + startIdx == selectedIndex) {
            u8g2.drawStr(x, currentY, ">");  // Selection arrow
            u8g2.drawStr(x + 8, currentY, items[i + startIdx]);
        } else {
            u8g2.drawStr(x + 8, currentY, items[i + startIdx]);
        }
    }

    u8g2.sendBuffer();
}

U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C* Screen::getDisplay() { return &u8g2; }