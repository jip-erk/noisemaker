#include "TextHeader.h"

TextHeader::TextHeader() : _screen(nullptr), _headerText("") {}

TextHeader::TextHeader(Screen *screen) : _screen(screen), _headerText("") {}

TextHeader::TextHeader(Screen *screen, const char *headerText)
    : _screen(screen), _headerText(headerText) {}

void TextHeader::setHeaderText(const char *text) { _headerText = text; }

void TextHeader::drawHeader() {
    if (!_screen) return;

    // Draw header text centered
    if (_headerText && strlen(_headerText) > 0) {
        int textWidth =
            strlen(_headerText) * 6;  // Assuming 6 pixels per character
        int x = (_screen->getWidth() - textWidth) / 2;
        _screen->drawStr(x, 2, _headerText);
    }
}