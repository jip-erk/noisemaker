#ifndef SampleManagerScreen_h
#define SampleManagerScreen_h

#include <Arduino.h>
#include "../Screen.h"

class SampleManagerScreen {
   public:
    SampleManagerScreen();
    SampleManagerScreen(Screen* screen);
    ~SampleManagerScreen();

    void drawList(const String* fileList, int selectedIndex, int fileCount);

   private:
    Screen* _screen;
};

#endif
