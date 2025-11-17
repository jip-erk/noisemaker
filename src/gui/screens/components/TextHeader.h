#ifndef TextHeader_h
#define TextHeader_h

#include <Arduino.h>

#include "../../Screen.h"

class TextHeader {
   public:
    TextHeader();
    TextHeader(Screen *screen);
    TextHeader(Screen *screen, const char *headerText);

    void setHeaderText(const char *text);
    void drawHeader();

   private:
    Screen *_screen;
    const char *_headerText;
};

#endif