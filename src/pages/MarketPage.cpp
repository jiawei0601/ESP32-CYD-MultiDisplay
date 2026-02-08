#include "MarketPage.h"

void MarketPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    // Initial update
    updateMarket();
}

void MarketPage::loop() {
    if (millis() - _lastUpdate > _interval) {
        updateMarket();
        draw();
    }
}

void MarketPage::draw() {
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);

    // NASDAQ Section
    _tft->setTextDatum(TL_DATUM);
    _tft->drawString("NASDAQ: ", 20, 40, 4);
    
    char nqStr[20];
    sprintf(nqStr, "%.2f", _nasdaqPrice);
    _tft->drawString(nqStr, 140, 40, 4);
    
    // BTC Section
    _tft->drawString("BTC/USDT: ", 20, 100, 4);
    char btcStr[20];
    sprintf(btcStr, "%.2f", _btcPrice);
    _tft->drawString(btcStr, 140, 100, 4);
}

void MarketPage::updateMarket() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // NASDAQ (Yahoo)
        String nasdaqUrl = "https://query1.finance.yahoo.com/v8/finance/chart/%5EIXIC?interval=1d&range=1d"; 
        http.begin(nasdaqUrl);
        if (http.GET() == 200) {
           JsonDocument doc;
           deserializeJson(doc, http.getString());
           _nasdaqPrice = doc["chart"]["result"][0]["meta"]["regularMarketPrice"];
        }
        http.end();

        // BTC (Binance)
        String btcUrl = "https://api.binance.com/api/v3/ticker/price?symbol=BTCUSDT";
        http.begin(btcUrl);
        if (http.GET() == 200) {
            JsonDocument doc;
            deserializeJson(doc, http.getString());
            _btcPrice = doc["price"].as<double>();
        }
        http.end();

        _lastUpdate = millis();
    }
}
