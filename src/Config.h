#ifndef CONFIG_H
#define CONFIG_H

// WiFi Config (Used as fallback or initial)
static const char* WIFI_SSID = "Your_SSID";
static const char* WIFI_PASS = "Your_PASSWORD";

// API Keys
static const char* OWM_API_KEY = "Your_OWM_Key"; // OpenWeatherMap
static const char* OWM_CITY = "Taipei,TW";

// Taiwan Stock Index Update Interval
static const unsigned long TW_STOCK_REFRESH = 10000; // 10 seconds

// NASDAQ Refresh
static const unsigned long NASDAQ_REFRESH = 60000; // 1 minute

// BTC Refresh
static const unsigned long BTC_REFRESH = 5000; // 5 seconds

#endif
