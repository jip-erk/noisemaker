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

    int getWidth();

    void drawItemList(int x, int y, const char* items[], int selectedIndex);
    void drawBox(int x, int y, int w, int h);
    void drawStr(int x, int y, const char* text);
    void setHeaderFont();
    void setNormalFont();

   private:
    U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C u8g2;  // Move u8g2 inside class
};

#endif