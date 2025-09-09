#ifndef Screen_h
#define Screen_h

#include <U8g2lib.h>

class Screen {
   public:
    Screen();
    void begin();
    void clear();
    void display();
    U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C* getDisplay();

    void drawItemList(int x, int y, const char* items[], int selectedIndex);

    typedef struct area {
        int x1;
        int y1;
        int x2;
        int y2;
    } Area;

    Area AREA_SCREEN = {0, 0, 128, 64};  // Fullscreen

    void drawStr(int x, int y, const char* text);

   private:
    U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C u8g2;  // Move u8g2 inside class
};

#endif