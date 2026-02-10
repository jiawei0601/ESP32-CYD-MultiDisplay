#include "WeatherPage.h"
#include <Preferences.h>
#include "../Style.h"

void WeatherPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    updateWeather();
}

void WeatherPage::loop() {
    // 檢查是否有新的城市設定
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 2000) {
        lastCheck = millis();
        Preferences prefs;
        prefs.begin("weather_v3", true);
        String currentSafeCity = prefs.getString("city", "Taipei");
        prefs.end();
        
        if (currentSafeCity != _lastSavedCity) {
            _lastUpdate = 0; // 強制重刷
            updateWeather();
            draw();
        }
    }

    // 更新邏輯
    if (_lastUpdate == 0 || millis() - _lastUpdate > _interval) {
        updateWeather(); 
        draw(); 
        if (_lastUpdate == 0) {
             _lastUpdate = millis() - (_interval - 15000); // 失敗的話 15 秒後再試
        }
    }
}

// 輔助繪圖函數：簡單的天氣圖示
void drawSun(TFT_eSPI* tft, int x, int y) {
    tft->fillCircle(x, y, 10, TFT_YELLOW);
    for (int i = 0; i < 360; i += 45) {
        float rad = i * DEG_TO_RAD;
        tft->drawLine(x + cos(rad)*12, y + sin(rad)*12, x + cos(rad)*18, y + sin(rad)*18, TFT_YELLOW);
    }
}

void drawCloud(TFT_eSPI* tft, int x, int y) {
    tft->fillCircle(x - 6, y, 6, TFT_LIGHTGREY);
    tft->fillCircle(x + 6, y, 6, TFT_LIGHTGREY);
    tft->fillCircle(x, y - 4, 8, TFT_LIGHTGREY);
}

void drawRain(TFT_eSPI* tft, int x, int y) {
    drawCloud(tft, x, y - 4);
    for (int i = -6; i <= 6; i += 6) {
        tft->drawLine(x + i, y + 4, x + i - 1, y + 10, TT_CYAN);
    }
}

void WeatherPage::draw() {
    _tft->fillRect(0, 25, 320, 215, TT_BG_COLOR);
    _tft->drawFastHLine(0, 25, 320, TT_BORDER_COLOR);

    _tft->setTextDatum(TC_DATUM);

    if (_temp == 0 && _lastUpdate < millis()) { 
        _tft->setTextColor(TFT_YELLOW, TT_BG_COLOR);
        _tft->drawString("Syncing Weather Data...", 160, 100, 2);
        
        // 顯示偵錯狀態
        _tft->setTextColor(TFT_DARKGREY, TT_BG_COLOR);
        _tft->drawString(_weatherDesc, 160, 130, 2); 
    } else {
        int cardY = 40;
        _tft->drawRoundRect(10, cardY, 300, 150, 8, TT_BORDER_COLOR);
        
        _tft->setTextColor(TT_CYAN);
        _tft->drawString(_city, 160, cardY + 10, 4);
        
        if (_weatherDesc.indexOf("Clear") >= 0) drawSun(_tft, 65, cardY + 60);
        else if (_weatherDesc.indexOf("Rain") >= 0) drawRain(_tft, 65, cardY + 60);
        else drawCloud(_tft, 65, cardY + 60);

        char tempStr[20];
        sprintf(tempStr, "%.1f C", _temp);
        _tft->setTextColor(TFT_WHITE);
        _tft->drawString(tempStr, 180, cardY + 45, 6);

        _tft->setTextColor(TFT_LIGHTGREY);
        _tft->drawString(_weatherDesc, 180, cardY + 95, 2);

        _tft->setTextDatum(TL_DATUM);
        _tft->setTextColor(TT_CYAN);
        _tft->setCursor(40, cardY + 120);
        _tft->print("Feels: "); _tft->print(_feelsLike, 1);
        _tft->setCursor(170, cardY + 120);
        _tft->print("Wind: "); _tft->print(_windSpeed, 1);
    }

    _tft->fillRect(0, 220, 320, 20, 0x1084);
    _tft->setTextDatum(TC_DATUM);
    _tft->setTextColor(TFT_DARKGREY);
    _tft->drawString("Aura Aesthetic Engine", 160, 223, 1);
}

void WeatherPage::updateWeather() {
    Preferences prefs;
    prefs.begin("weather_v3", true); 
    float lat = prefs.getFloat("lat", 25.03); 
    float lon = prefs.getFloat("lon", 121.56);
    _city = prefs.getString("city", "Taipei");
    _lastSavedCity = _city;
    prefs.end();

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = "http://api.open-meteo.com/v1/forecast?latitude=" + String(lat, 2) + 
                     "&longitude=" + String(lon, 2) + 
                     "&current_weather=true";
        
        http.begin(url);
        http.setUserAgent("ESP32-Aura/1.0"); // 增加 User-Agent
        http.setTimeout(10000); // 增加超時時間至 10 秒
        
        int httpCode = http.GET();
        if (httpCode == 200) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                _temp = doc["current_weather"]["temperature"];
                _windSpeed = doc["current_weather"]["windspeed"]; 
                int code = doc["current_weather"]["weathercode"];
                
                if (code == 0) _weatherDesc = "Clear Sky";
                else if (code <= 3) _weatherDesc = "Cloudy";
                else if (code >= 51 && code <= 67) _weatherDesc = "Rainy";
                else _weatherDesc = "Overcast";
                
                _feelsLike = _temp - 0.4;
                _lastUpdate = millis();
                Serial.printf("[Weather] Success: %.1f C\n", _temp);
            } else {
                _weatherDesc = "JSON Error";
                Serial.printf("[Weather] JSON Fail: %s\n", error.c_str());
            }
        } else {
            _weatherDesc = "HTTP: " + String(httpCode);
            Serial.printf("[Weather] HTTP Fail: %d\n", httpCode);
        }
        http.end();
    } else {
        _weatherDesc = "WiFi Disconnected";
    }
}
