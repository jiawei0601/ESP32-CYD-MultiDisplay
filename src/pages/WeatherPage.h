#ifndef WEATHER_PAGE_H
#define WEATHER_PAGE_H

#include "../Page.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../Config.h"

class WeatherPage : public Page {
public:
    void setup(TFT_eSPI* tft) override;
    void loop() override;
    void draw() override;
    const char* getName() override { return "Weather"; }

private:
    TFT_eSPI* _tft;
    unsigned long _lastUpdate = 0;
    const unsigned long _interval = 600000; // 10 minutes
    String _city;
    String _weatherDesc;
    float _temp;
    int _humidity;
    
    void updateWeather();
};

#endif
