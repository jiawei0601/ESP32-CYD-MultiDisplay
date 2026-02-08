#ifndef STOCK_PAGE_H
#define STOCK_PAGE_H

#include "../Page.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../Config.h"

class StockPage : public Page {
public:
    void setup(TFT_eSPI* tft) override;
    void loop() override;
    void draw() override;
    const char* getName() override { return "Stock"; }

private:
    TFT_eSPI* _tft;
    unsigned long _lastUpdate = 0;
    const unsigned long _interval = 10000; // 10s
    String _symbol = "^TWII"; // TAIEX or specific stock
    double _price = 0.0;
    double _change = 0.0;
    
    void updateStock();
};

#endif
