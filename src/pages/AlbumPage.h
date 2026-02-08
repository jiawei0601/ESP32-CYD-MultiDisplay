#ifndef ALBUM_PAGE_H
#define ALBUM_PAGE_H

#include "../Page.h"
#include <SD.h>
#include <TJpg_Decoder.h>

class AlbumPage : public Page {
public:
    void setup(TFT_eSPI* tft) override;
    void loop() override;
    void draw() override;
    const char* getName() override { return "Album"; }

private:
    TFT_eSPI* _tft;
    File _root;
    File _currentFile;
    unsigned long _lastUpdate = 0;
    const unsigned long _interval = 5000; // 5s per photo

    void nextImage();
};

#endif
