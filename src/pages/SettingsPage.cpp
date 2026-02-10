#include "SettingsPage.h"

// External access to touch (from main.cpp or passed in setup, but we use global for now to keep it simple or fix main loop to pass touch)
// Ideally Page::loop() should take touch as argument, but for now we'll assume touch is handled or we use a global pointer if needed.
// Actually, main loop calls handleTouch() logic for page switching. We need page-specific touch.
// Let's rely on the fact that `touch` object is global in main.cpp, we can declare `extern`.
#include <XPT2046_Touchscreen.h>
extern XPT2046_Touchscreen touch;

void SettingsPage::setup(TFT_eSPI* tft) {
    _tft = tft;
    prefs.begin("wifi-config", false); // Namespace "wifi-config"
    initKeyboard();
    scanNetworks();
}

void SettingsPage::loop() {
    handleTouch();
}

void SettingsPage::draw() {
    _tft->fillScreen(TFT_BLACK);
    
    if (_state == STATE_SCAN) {
        drawScanList();
    } else if (_state == STATE_INPUT_PASS) {
        drawKeyboard();
    } else if (_state == STATE_CONNECTING) {
        _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        _tft->drawCentreString("Connecting to:", 160, 80, 4);
        _tft->drawCentreString(_ssids[_selectedNetworkIndex], 160, 120, 4);
        _tft->drawCentreString("Please Wait...", 160, 160, 4);
    } else if (_state == STATE_SELECT_CITY) {
        drawCityList();
    }
}

void SettingsPage::scanNetworks() {
    _state = STATE_SCAN;
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawCentreString("Scanning WiFi...", 160, 120, 4);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    _networkCount = WiFi.scanNetworks();
    
    _ssids.clear();
    for (int i = 0; i < _networkCount && i < 6; ++i) { // Limit to 6
        _ssids.push_back(WiFi.SSID(i));
    }
    draw();
}

void SettingsPage::drawScanList() {
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawCentreString("Select WiFi Network", 160, 10, 2);
    _tft->drawFastHLine(0, 30, 320, TFT_BLUE);

    for (int i = 0; i < _ssids.size(); i++) {
        int y = 40 + i * 35;
        _tft->drawRect(10, y, 300, 30, TFT_DARKGREY);
        _tft->drawString(_ssids[i], 20, y + 5, 2);
        
        // Signal strength bar (mock or real if we saved RSSI)
        // _tft->drawString(String(WiFi.RSSI(i)), 280, y+5, 2);
    }
    
    // Rescan Button
    _tft->fillRect(20, 210, 120, 25, TFT_DARKGREEN);
    _tft->setTextColor(TFT_WHITE);
    _tft->drawCentreString("Rescan", 80, 215, 2);

    // Weather City Button
    _tft->fillRect(180, 210, 120, 25, TFT_BLUE);
    _tft->drawCentreString("Set City", 240, 215, 2);
}

void SettingsPage::initKeyboard() {
    _keys.clear();
    int startX = 5;
    int startY = 80;
    int keyW = 30;
    int keyH = 30;
    int gap = 2;
    
    const char* rows[] = {
        "1234567890",
        "QWERTYUIOP",
        "ASDFGHJKL",
        "ZXCVBNM"
    };
    
    // Row 1-4
    for (int r = 0; r < 4; r++) {
        int len = strlen(rows[r]);
        int rowX = startX + (320 - (len * (keyW + gap))) / 2;
        for (int c = 0; c < len; c++) {
            Key k;
            k.x = rowX + c * (keyW + gap);
            k.y = startY + r * (keyH + gap);
            k.w = keyW; k.h = keyH;
            k.value = rows[r][c];
            sprintf(k.label, "%c", k.value);
            _keys.push_back(k);
        }
    }
    
    // Special Keys
    Key kBack = {240, 200, 70, 30, "<-", 8};   // Backspace
    Key kEnter = {10, 200, 70, 30, "OK", 13};  // Enter
    Key kCancel = {100, 200, 70, 30, "X", 27}; // ESC/Cancel
    
    _keys.push_back(kBack);
    _keys.push_back(kEnter);
    _keys.push_back(kCancel);
}

void SettingsPage::drawKeyboard() {
    _tft->fillScreen(TFT_BLACK);
    
    // Input Box
    _tft->drawRect(10, 40, 300, 30, TFT_WHITE);
    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->drawString(_password, 15, 45, 4);
    
    // Keys
    _tft->setTextDatum(MC_DATUM);
    for (auto &k : _keys) {
        _tft->fillRect(k.x, k.y, k.w, k.h, TFT_DARKGREY);
        _tft->drawRect(k.x, k.y, k.w, k.h, TFT_WHITE);
        _tft->setTextColor(TFT_WHITE);
        _tft->drawString(k.label, k.x + k.w/2, k.y + k.h/2, 2);
    }
}

void SettingsPage::connectWiFi() {
    _state = STATE_CONNECTING;
    draw();
    
    WiFi.begin(_ssids[_selectedNetworkIndex].c_str(), _password.c_str());
    
    unsigned long start = millis();
    bool connected = false;
    while(millis() - start < 15000) {
        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            break;
        }
        delay(100);
    }
    
    if (connected) {
        prefs.putString("ssid", _ssids[_selectedNetworkIndex]);
        prefs.putString("pass", _password);
        _tft->fillScreen(TFT_GREEN);
        _tft->setTextColor(TFT_BLACK);
        _tft->drawCentreString("Connected!", 160, 120, 4);
        delay(2000);
        // Maybe return to main page?
        scanNetworks(); // Go back to list
    } else {
        _tft->fillScreen(TFT_RED);
        _tft->setTextColor(TFT_WHITE);
        _tft->drawCentreString("Failed!", 160, 120, 4);
        delay(2000);
        _state = STATE_INPUT_PASS; // Retry password
        draw();
    }
}

void SettingsPage::handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, 3550, 350, 0, 320); 
    int ty = map(p.y, 3750, 350, 0, 240); 
    tx = constrain(tx, 0, 319); 
    ty = constrain(ty, 0, 239);

    if (_state == STATE_SCAN) {
        // WiFi List Check
        for (int i = 0; i < _ssids.size(); i++) {
            int y = 40 + i * 35;
            if (tx > 10 && tx < 310 && ty > y && ty < y + 30) {
                _selectedNetworkIndex = i;
                _password = "";
                _state = STATE_INPUT_PASS;
                draw();
                delay(400);
                return;
            }
        }
        // Rescan Button
        if (ty > 210 && ty < 240 && tx > 20 && tx < 140) {
            scanNetworks();
            delay(400);
        }
        // Set City Button
        if (ty > 210 && ty < 240 && tx > 180 && tx < 300) {
            _state = STATE_SELECT_CITY;
            draw();
            delay(400);
        }
    } 
    else if (_state == STATE_INPUT_PASS) {
        for (auto &k : _keys) {
            if (tx >= k.x && tx <= k.x + k.w && ty >= k.y && ty <= k.y + k.h) {
                if (k.value == 8) { // Backspace
                    if (_password.length() > 0) _password.remove(_password.length()-1);
                } else if (k.value == 13) { // Enter
                    connectWiFi();
                    return; 
                } else if (k.value == 27) { // Cancel
                    _state = STATE_SCAN;
                    draw();
                    return;
                } else {
                    _password += k.value;
                }
                drawKeyboard();
                delay(250); 
                return;
            }
        }
    } 
    else if (_state == STATE_SELECT_CITY) {
        // 在點擊處畫紅點偵錯
        _tft->fillCircle(tx, ty, 3, TFT_RED);
        
        // 輸出原始數據與映射數據
        Serial.printf("TOUCH RAW [%d,%d] -> MAP [%d,%d]\n", p.x, p.y, tx, ty);
        
        // 偵錯顯示：座標數值
        char buf[32];
        sprintf(buf, "X:%d Y:%d", tx, ty);
        _tft->fillRect(0, 25, 100, 15, TFT_BLACK);
        _tft->setTextColor(TFT_YELLOW, TFT_BLACK);
        _tft->drawString(buf, 10, 28, 1);

        struct CityItem { String name; float lat; float lon; };
        CityItem cities[] = {
            {"Taipei", 25.03, 121.56},
            {"Hsinchu", 24.81, 120.97},
            {"Taichung", 24.14, 120.67},
            {"Tainan", 22.99, 120.21},
            {"Kaohsiung", 22.62, 120.31},
            {"Hualien", 23.97, 121.60}
        };

        // 重新計算判定：i 從 0 到 5，y 從 35 開始，每項 29 像素以縮短間距
        for (int i = 0; i < 6; i++) {
            int yStart = 35 + i * 29;
            if (tx > 5 && tx < 315 && ty > yStart && ty < yStart + 27) {
                saveCity(cities[i].name, cities[i].lat, cities[i].lon);
                while(touch.touched()); // 等放開
                draw(); // 刷新綠框
                return;
            }
        }
        
        // Buttons
        if (ty > 210 && ty < 240) {
            if (tx > 180) { // Back
                while(touch.touched());
                _state = STATE_SCAN;
                draw();
            } else if (tx < 140) { // Auto
                while(touch.touched());
                autoLocate();
            }
        }
    }
}

void SettingsPage::autoLocate() {
    if (WiFi.status() == WL_CONNECTED) {
        _tft->fillScreen(TFT_BLACK);
        _tft->setTextColor(TFT_WHITE);
        _tft->drawCentreString("Locating via IP...", 160, 120, 2);
        
        HTTPClient http;
        http.begin("http://ip-api.com/json");
        int httpCode = http.GET();
        
        if (httpCode == 200) {
            JsonDocument doc;
            deserializeJson(doc, http.getString());
            
            String city = doc["city"].as<String>();
            float lat = doc["lat"];
            float lon = doc["lon"];
            
            saveCity(city, lat, lon);
            
            _tft->fillScreen(TFT_GREEN);
            _tft->setTextColor(TFT_BLACK);
            _tft->drawCentreString("AUTO SUCCESS!", 160, 100, 2);
            _tft->drawCentreString(city, 160, 130, 4);
            delay(2000);
        } else {
            _tft->fillScreen(TFT_RED);
            _tft->drawCentreString("Location Failed", 160, 120, 2);
            delay(2000);
        }
        _state = STATE_SCAN;
        draw();
    }
}

void SettingsPage::drawCityList() {
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TFT_CYAN, TFT_BLACK);
    _tft->drawCentreString("WEATHER LOCATION", 160, 5, 2);
    _tft->drawFastHLine(0, 25, 320, TFT_BLUE);

    Preferences cityPrefs;
    cityPrefs.begin("weather_v3", true);
    String cur = cityPrefs.getString("city", "Taipei");
    cityPrefs.end();

    String names[] = {"Taipei", "Hsinchu", "Taichung", "Tainan", "Kaohsiung", "Hualien"};
    for (int i = 0; i < 6; i++) {
        int y = 35 + i * 29;
        bool active = (names[i] == cur);
        
        if (active) {
            _tft->drawRect(10, y, 300, 27, TFT_GREEN);
            _tft->setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
            _tft->drawRect(10, y, 300, 27, TFT_DARKGREY);
            _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        }
        _tft->drawCentreString(names[i], 160, y + 6, 2);
    }

    // Bottom Buttons
    _tft->fillRect(20, 212, 120, 24, TFT_BLUE);
    _tft->setTextColor(TFT_WHITE);
    _tft->drawCentreString("AUTO GPS", 80, 216, 2);

    _tft->fillRect(180, 212, 120, 24, TFT_DARKGREY);
    _tft->drawCentreString("BACK", 240, 216, 2);
}

void SettingsPage::saveCity(String name, float lat, float lon) {
    _tft->fillScreen(TFT_BLACK);
    _tft->setTextColor(TFT_GREEN);
    _tft->drawCentreString("CONFIRMED & SAVED", 160, 100, 2);
    _tft->setTextColor(TFT_WHITE);
    _tft->drawCentreString(name, 160, 130, 4);

    Preferences cityPrefs;
    cityPrefs.begin("weather_v3", false); 
    cityPrefs.putFloat("lat", lat);
    cityPrefs.putFloat("lon", lon);
    cityPrefs.putString("city", name);
    cityPrefs.end();
    
    Serial.printf("CITY SAVED: %s (%.2f, %.2f)\n", name.c_str(), lat, lon);
    delay(1500); 
}
