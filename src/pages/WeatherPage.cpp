#include "WeatherPage.h"

void WeatherPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    updateWeather();
}

void WeatherPage::loop() {
    if (millis() - _lastUpdate > _interval) {
        updateWeather(); 
        draw(); 
    }
}

void WeatherPage::draw() {
    _tft->fillScreen(TFT_BLACK);
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
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(OWM_CITY) + "&appid=" + String(OWM_API_KEY) + "&units=metric";
        
        http.begin(url);
        int httpCode = http.GET();
        
        if (httpCode > 0) {
            String payload = http.getString();
            JsonDocument doc;
            deserializeJson(doc, payload);
            
            _temp = doc["main"]["temp"];
            _humidity = doc["main"]["humidity"];
            _weatherDesc = doc["weather"][0]["main"].as<String>();
            _city = doc["name"].as<String>();
            _lastUpdate = millis();
        }
        http.end();
    }
}
