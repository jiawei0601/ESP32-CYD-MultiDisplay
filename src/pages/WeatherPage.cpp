#include "WeatherPage.h"
#include <Preferences.h>

void WeatherPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    updateWeather();
}

void WeatherPage::loop() {
    // 檢查是否有新的城市設定 (每秒檢查一次即可)
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 2000) {
        lastCheck = millis();
        Preferences prefs;
        prefs.begin("weather_v3", true);
        String currentSafeCity = prefs.getString("city", "Taipei");
        prefs.end();
        
        // 如果發現設定改了，立即觸發更新
        if (currentSafeCity != _lastSavedCity) {
            updateWeather();
            draw();
        }
    }

    if (millis() - _lastUpdate > _interval) {
        updateWeather(); 
        draw(); 
    }
}

void WeatherPage::draw() {
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    
    // Title
    _tft->setTextDatum(TC_DATUM);
    _tft->drawString("Weather: " + _city, 160, 40, 4);

    // Weather Data
    _tft->setTextDatum(MC_DATUM);
    _tft->drawString(_weatherDesc, 160, 100, 4);
    
    char tempStr[20];
    sprintf(tempStr, "%.1f C", _temp);
    _tft->drawString(tempStr, 160, 140, 6);

    char humStr[20];
    sprintf(humStr, "Hum: %d %%", _humidity);
    _tft->drawString(humStr, 160, 190, 4);
}


void WeatherPage::updateWeather() {
    Preferences prefs;
    prefs.begin("weather_v3", true); 
    float lat = prefs.getFloat("lat", 25.03); 
    float lon = prefs.getFloat("lon", 121.56);
    _city = prefs.getString("city", "Taipei");
    _lastSavedCity = _city; // 更新紀錄標記
    prefs.end();

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        // 改用 http 避免 SSL 證書導致的請求失敗
        String url = "http://api.open-meteo.com/v1/forecast?latitude=" + String(lat) + "&longitude=" + String(lon) + "&current_weather=true";
        
        http.begin(url);
        http.setTimeout(5000);
        int httpCode = http.GET();
        
        if (httpCode == 200) {
            String payload = http.getString();
            JsonDocument doc;
            deserializeJson(doc, payload);
            
            _temp = doc["current_weather"]["temperature"];
            _humidity = doc["current_weather"]["windspeed"]; 
            
            int code = doc["current_weather"]["weathercode"];
            
            if (code == 0) _weatherDesc = "Clear Sky";
            else if (code >= 1 && code <= 3) _weatherDesc = "Partly Cloudy";
            else if (code >= 45 && code <= 48) _weatherDesc = "Foggy";
            else if (code >= 51 && code <= 67) _weatherDesc = "Rainy";
            else if (code >= 71 && code <= 77) _weatherDesc = "Snowy";
            else if (code >= 80 && code <= 99) _weatherDesc = "Stormy";
            else _weatherDesc = "Cloudy";

            _lastUpdate = millis();
        } else {
            _weatherDesc = "Connect Error";
        }
        http.end();
    } else {
        _weatherDesc = "No WiFi";
    }
}
