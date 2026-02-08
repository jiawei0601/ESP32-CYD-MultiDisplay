#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include "Config.h"
#include "Page.h"

// Page Includes
// #include "pages/WeatherPage.h"
// #include "pages/StockPage.h"
// #include "pages/MarketPage.h"
// #include "pages/AlbumPage.h"

// Hardware
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen touch(33, 36); // CS, IRQ for CYD

// Pages
const int PAGE_COUNT = 4;
Page* pages[PAGE_COUNT];
int currentPage = 0;

void switchPage(int index);

// UI
void drawTopBar() {
    tft.fillRect(0, 0, 320, 24, TFT_NAVY);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    for(int i=0; i<PAGE_COUNT; i++) {
        int w = 320 / PAGE_COUNT;
        int x = i * w;
        if(i == currentPage) tft.fillRect(x, 0, w, 24, TFT_BLUE);
        // tft.drawString(pages[i]->getName(), x + w/2, 12, 2);
        char label[2]; sprintf(label, "%d", i+1);
        tft.drawString(label, x + w/2, 12, 2);
    }
}

void setup() {
    Serial.begin(115200);

    // Display Init
    tft.init();
    tft.setRotation(1); 
    tft.invertDisplay(true); // CYD sometimes needs this
    tft.fillScreen(TFT_BLACK);

    // Touch
    // touch.begin(); 

    // WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    tft.drawString("Connecting WiFi...", 160, 120, 4);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");

    // Initialize Pages
    // pages[0] = new WeatherPage();
    // pages[1] = new AlbumPage();
    // pages[2] = new StockPage();
    // pages[3] = new MarketPage(); // Include NASDAQ & BTC here

    /* for(int i=0; i<PAGE_COUNT; i++) {
        pages[i]->setup(&tft);
    } */

    switchPage(0);
}

void loop() {
    // Touch Check for Top Bar
    /*
    if (touch.touched()) {
        TS_Point p = touch.getPoint();
        // Calibrate & Map p.x, p.y to screen
        // If y < 30, check x range for page switch
    }
    */
    
    // pages[currentPage]->loop();
    delay(10);
}

void switchPage(int index) {
    if(index < 0 || index >= PAGE_COUNT) return;
    currentPage = index;
    // pages[currentPage]->draw();
    drawTopBar();
}
