#include "StockPage.h"

void StockPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    updateStock();
}

void StockPage::loop() {
    if (millis() - _lastUpdate > _interval) {
        updateStock(); 
        draw(); 
    }
}

void StockPage::draw() {
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    
    // Header
    _tft->drawString("TWSE Index", 160, 40, 4);

    // Price
    char priceStr[20];
    sprintf(priceStr, "%.2f", _price);
    _tft->drawString(priceStr, 160, 100, 7); // Large font
    
    // Change
    uint32_t color = (_change >= 0) ? TFT_RED : TFT_GREEN; // TW color convention (Red is Up)
    _tft->setTextColor(color, TFT_BLACK);
    char changeStr[20];
    sprintf(changeStr, "%+.2f", _change);
    _tft->drawString(changeStr, 160, 160, 4);
    
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString("Updated", 160, 220, 2);
}

void StockPage::updateStock() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        // Use a proxy or direct Yahoo query. Direct Yahoo is hard due to cookies.
        // For demonstration, use a placeholder or public simple API if available.
        // In real scenario, we might need a more complex scraper.
        // Here, we simulate with a random walk from base if API fails,
        // or attempt a known simple JSON endpoint if one existed.
        // User asked for "Implement", so I should try a real endpoint.
        // Global Quote API from Yahoo (requires key usually) or Finnhub.
        // Let's use a mock implementation for "Real Time" updates if API fails,
        // but try to fetch from a source if possible.
        // Actually, for TWSE, standard HTTP GET to mis.twse is possible but complex parsing.
        
        // I will use a simple "Mock" approach here because without a guaranteed API key/proxy, 
        // the code will just fail. I'll add a comment about where to plug the API.
        
        // Mock Data for "10s refresh" demo
        // _price += (random(-100, 100) / 100.0);
        // _change = _price - 15000;
        
        // Real approach:
        // Use standard Yahoo Finance chart API which returns JSON
        String url = "https://query1.finance.yahoo.com/v8/finance/chart/" + _symbol + "?interval=1d&range=1d"; 
        // This is often open.
        
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode == 200) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                float regularMarketPrice = doc["chart"]["result"][0]["meta"]["regularMarketPrice"];
                float previousClose = doc["chart"]["result"][0]["meta"]["chartPreviousClose"];
                _price = regularMarketPrice;
                _change = _price - previousClose;
                _lastUpdate = millis();
            }
        }
        http.end();
    }
}
