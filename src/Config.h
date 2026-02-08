#ifndef CONFIG_H
#define CONFIG_H

// WiFi Config
const char* WIFI_SSID = "Your_SSID";
const char* WIFI_PASS = "Your_PASSWORD";

// API Keys
const char* OWM_API_KEY = "Your_OWM_Key"; // OpenWeatherMap
const char* OWM_CITY = "Taipei,TW";

// Taiwan Stock Index Update Interval
const unsigned long TW_STOCK_REFRESH = 10000; // 10 seconds

// NASDAQ Refresh
const unsigned long NASDAQ_REFRESH = 60000; // 1 minute

// BTC Refresh
const unsigned long BTC_REFRESH = 5000; // 5 seconds

#endif
