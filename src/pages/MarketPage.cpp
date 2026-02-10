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
    // 保留頂部導航欄，只清除下方區域
    _tft->fillRect(0, 25, 320, 215, TFT_BLACK);
    _tft->setTextColor(TT_CYAN, TFT_BLACK);
    _tft->drawCentreString("MARKET TICKER", 160, 32, 2);
    _tft->drawFastHLine(0, 48, 320, TT_BORDER_COLOR);

    int w = 158;
    int h = 88; // 稍微縮短高度以適應下移
    drawCell(0, 1, 52, w, h);
    drawCell(1, 161, 52, w, h);
    drawCell(2, 1, 142, w, h);
    drawCell(3, 161, 142, w, h);
}

void MarketPage::drawCell(int idx, int x, int y, int w, int h) {
    MarketAsset& a = _assets[idx];
    _tft->drawRect(x, y, w, h, TT_BORDER_COLOR);
    
    if (!a.isValid) {
        _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        _tft->drawCentreString("Loading...", x + w/2, y + h/2 - 12, 2);
        
        // 顯示最後一次的錯誤狀態以便偵錯
        _tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
        _tft->drawCentreString(_lastError, x + w/2, y + h/2 + 8, 1);
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
        bool allOk = true;
        for (int i = 0; i < 4; i++) {
            String sym = _assets[i].symbol;
            String url = "https://brapi.dev/api/quote/" + sym; 
            if (sym == "^IXIC") url = "https://brapi.dev/api/quote/%5EIXIC"; 

            http.begin(url);
            http.setUserAgent("ESP32-Ticker");
            http.setTimeout(8000);
            
            int httpCode = http.GET();
            if (httpCode == 200) {
                String payload = http.getString();
                JsonDocument doc;
                deserializeJson(doc, payload);
                
                JsonObject data = doc["results"][0];
                _assets[i].price = data["regularMarketPrice"];
                
                // 具備容錯性的漲跌幅抓取
                if (data.containsKey("regularMarketChange")) {
                    _assets[i].change = data["regularMarketChange"];
                    _assets[i].changePercent = data["regularMarketChangePercent"];
                } else if (data.containsKey("change")) {
                    _assets[i].change = data["change"];
                    _assets[i].changePercent = data["changePercent"];
                }
                
                _assets[i].isValid = true;
                _lastError = "OK";
            } else {
                // Yahoo 備援路徑補強
                String backupUrl = "https://query1.finance.yahoo.com/v8/finance/chart/" + sym + "?interval=1d&range=1d";
                http.begin(backupUrl);
                int bCode = http.GET();
                if (bCode == 200) {
                    JsonDocument doc;
                    deserializeJson(doc, http.getString());
                    if (doc["chart"]["result"].size() > 0) {
                        JsonObject meta = doc["chart"]["result"][0]["meta"];
                        _assets[i].price = meta["regularMarketPrice"];
                        double prevClose = meta["previousClose"];
                        if (prevClose == 0) prevClose = meta["chartPreviousClose"];
                        
                        _assets[i].change = _assets[i].price - prevClose;
                        _assets[i].changePercent = (prevClose != 0) ? (_assets[i].change / prevClose) * 100.0 : 0;
                        _assets[i].isValid = true;
                        _lastError = "Backup OK";
                    }
                } else {
                    allOk = false;
                    _lastError = "Err: " + String(httpCode);
                }
            }
            http.end();
            delay(300); 
        }
        if (allOk) _lastUpdate = millis();
    } else {
        _lastError = "No WiFi";
    }
}
