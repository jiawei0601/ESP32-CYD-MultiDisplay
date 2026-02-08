#ifndef PAGE_H
#define PAGE_H

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

class Page {
public:
    virtual void setup(TFT_eSPI* tft) = 0;
    virtual void loop() = 0;
    virtual void draw() = 0; // Full redraw
    virtual const char* getName() = 0; 
};

#endif
