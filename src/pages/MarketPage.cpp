#include "MarketPage.h"
#include "../Style.h"

void MarketPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    _assets[0].symbol = "^IXIC";   _assets[0].name = "NASDAQ";
    _assets[1].symbol = "NVDA";    _assets[1].name = "NVDA";
    _assets[2].symbol = "GOOGL";   _assets[2].name = "GOOGLE";
    _assets[3].symbol = "BTC-USD"; _assets[3].name = "BTC";
    updateMarket();
}

void MarketPage::loop() {
    if (millis() - _lastUpdate > _interval) {
        _lastUpdate = millis(); // 固定更新間隔
        updateMarket();
        draw();
    }
}

void MarketPage::draw() {
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TT_CYAN, TFT_BLACK);
    _tft->drawCentreString("MARKET TICKER", 160, 5, 2);
    _tft->drawFastHLine(0, 30, 320, TT_BORDER_COLOR);

    int w = 158;
    int h = 98;
    drawCell(0, 1, 35, w, h);
    drawCell(1, 161, 35, w, h);
    drawCell(2, 1, 137, w, h);
    drawCell(3, 161, 137, w, h);
}

void MarketPage::drawCell(int idx, int x, int y, int w, int h) {
    MarketAsset& a = _assets[idx];
    _tft->drawRect(x, y, w, h, TT_BORDER_COLOR);
    
    if (!a.isValid) {
        _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        _tft->drawCentreString("Loading...", x + w/2, y + h/2 - 8, 2);
        return;
    }

    uint32_t trendColor = (a.change >= 0) ? TFT_RED : TFT_GREEN;

    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawCentreString(a.name, x + w/2, y + 10, 4);

    _tft->setTextColor(trendColor, TFT_BLACK);
    char priceStr[20];
    sprintf(priceStr, "%.2f", a.price);
    _tft->drawCentreString(priceStr, x + w/2, y + 42, 4);

    char changeStr[40];
    sprintf(changeStr, "%+.2f (%.1f%%)", a.change, a.changePercent);
    _tft->drawCentreString(changeStr, x + w/2, y + 74, 2);
}

void MarketPage::updateMarket() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        for (int i = 0; i < 4; i++) {
            String sym = _assets[i].symbol;
            String originalSym = sym;
            sym.replace("^", "%5E");
            String url = "https://query1.finance.yahoo.com/v8/finance/chart/" + sym + "?interval=1d&range=1d"; 
            
            http.begin(url);
            http.setUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
            
            int httpCode = http.GET();
            if (httpCode == 200) {
                String payload = http.getString();
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload);
                if (!error && doc["chart"]["result"].size() > 0) {
                    JsonObject meta = doc["chart"]["result"][0]["meta"];
                    if (meta.containsKey("regularMarketPrice")) {
                        _assets[i].price = meta["regularMarketPrice"];
                        double prevClose = meta["chartPreviousClose"];
                        _assets[i].change = _assets[i].price - prevClose;
                        _assets[i].changePercent = (prevClose != 0) ? (_assets[i].change / prevClose) * 100.0 : 0;
                        _assets[i].isValid = true;
                        Serial.printf("[Market] %s Loaded: %.2f\n", originalSym.c_str(), _assets[i].price);
                    } else {
                        Serial.printf("[Market] %s: No Price data in JSON\n", originalSym.c_str());
                    }
                } else {
                    Serial.printf("[Market] %s: JSON Parse Error or Empty Result\n", originalSym.c_str());
                }
            } else {
                Serial.printf("[Market] %s Fetch Fail: %d\n", originalSym.c_str(), httpCode);
            }
            http.end();
            delay(200); // 增加延遲確保穩定
        }
        _lastUpdate = millis();
    }
}
