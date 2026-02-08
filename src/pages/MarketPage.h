#ifndef MARKET_PAGE_H
#define MARKET_PAGE_H

#include "../Page.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

class MarketPage : public Page {
public:
    void setup(TFT_eSPI* tft) override;
    void loop() override;
    void draw() override;
    const char* getName() override { return "Market"; }

private:
    TFT_eSPI* _tft;
    unsigned long _lastUpdate = 0;
    const unsigned long _interval = 10000; 

    // NASDAQ
    double _nasdaqPrice = 0;
    double _nasdaqChange = 0;
    
    // BTC
    double _btcPrice = 0;
    
    void updateMarket();
};

#endif
