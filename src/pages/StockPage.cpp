#include "StockPage.h"
#include "../Style.h"

void StockPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    updateStock();
}

void StockPage::loop() {
    if (millis() - _lastUpdate > _interval) {
        _lastUpdate = millis(); // 立即更新時間，防止 API 失敗導致的連續重刷
        updateStock(); 
        draw(); 
    }
}

void StockPage::draw() {
    _tft->fillScreen(TFT_BLACK);
    _tft->drawFastHLine(0, 30, 320, TT_BORDER_COLOR);
    _tft->setTextColor(TT_CYAN, TFT_BLACK);
    _tft->drawCentreString("SINGLE ASSET VIEW", 160, 5, 2);

    _tft->drawRect(10, 40, 300, 180, TT_BORDER_COLOR);

    _tft->setTextDatum(MC_DATUM);
    
    // Symbol
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawCentreString(_symbol, 160, 70, 4);

    // 決定字體顏色 (漲紅跌綠)
    uint32_t trendColor = (_change >= 0) ? TFT_RED : TFT_GREEN;

    // Price / Loading
    if (_price <= 0) {
        _tft->setTextColor(TFT_YELLOW, TFT_BLACK);
        _tft->drawCentreString("FETCHING DATA...", 160, 120, 4);
    } else {
        _tft->setTextColor(trendColor, TFT_BLACK);
        char priceStr[20];
        sprintf(priceStr, "%.2f", _price);
        _tft->drawCentreString(priceStr, 160, 120, 7);
        
        char changeStr[40];
        float changePercent = (_price > abs(_change)) ? (_change / (_price - _change)) * 100.0 : 0;
        sprintf(changeStr, "%+.2f (%.1f%%)", _change, changePercent);
        _tft->drawCentreString(changeStr, 160, 180, 4);
    }
    
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawCentreString("CYD STOCK TICKER", 160, 205, 2);
    _tft->setTextDatum(TL_DATUM); 
}

void StockPage::updateStock() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        // 使用網址安全編碼 %5E 代替 ^
        String url = "https://query1.finance.yahoo.com/v8/finance/chart/%5ETWII?interval=1d&range=1d"; 
        
        http.begin(url);
        http.setUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"); 
        
        int httpCode = http.GET();
        if (httpCode == 200) {
            JsonDocument doc;
            deserializeJson(doc, http.getString());
            if (doc["chart"]["result"].size() > 0) {
                JsonObject meta = doc["chart"]["result"][0]["meta"];
                _price = meta["regularMarketPrice"];
                double prevClose = meta["chartPreviousClose"];
                _change = _price - prevClose;
                _lastUpdate = millis();
                Serial.printf("TAIEX Loaded: %.2f\n", _price);
            }
        } else {
            Serial.printf("TAIEX Fetch Fail (HttpCode: %d)\n", httpCode);
        }
        http.end();
    }
}
