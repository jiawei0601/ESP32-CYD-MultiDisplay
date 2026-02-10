#include "AlbumPage.h"
#include <SPI.h>

// Global pointer for dirty callback hack if necessary, or use lambda
TFT_eSPI* g_tft = nullptr;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (y >= g_tft->height()) return 0;
    g_tft->pushImage(x, y, w, h, bitmap);
    return 1;
}

void AlbumPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    g_tft = tft;

    /* 暫時停用 SD 卡以解決 SPI 觸控衝突
    SPI.begin(18, 19, 23, 5); // CLK, MISO, MOSI, CS
    if (!SD.begin(5)) {
        Serial.println("SD Mount Failed");
        return;
    }
    */
    
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);

    _root = SD.open("/");
    nextImage();
}

void AlbumPage::loop() {
    if (millis() - _lastUpdate > _interval) {
        nextImage();
        _lastUpdate = millis();
    }
}

void AlbumPage::draw() {
    // Drawn via nextImage() directly to screen
}

void AlbumPage::nextImage() {
    File entry = _root.openNextFile();
    if (!entry) {
        _root.rewindDirectory();
        entry = _root.openNextFile();
    }
    
    if (entry) {
        String name = entry.name();
        if (name.endsWith(".jpg") || name.endsWith(".JPG")) {
            // Center image or fit
            TJpgDec.drawJpg(0, 24, entry.name()); // Offset for topbar
        }
        entry.close();
    }
}
