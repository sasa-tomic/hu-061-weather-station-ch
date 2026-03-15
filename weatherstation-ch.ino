/**The MIT License (MIT)

Copyright (c) 2018 by Daniel Eichhorn - ThingPulse

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at https://thingpulse.com
*/

#include <Arduino.h>

#include <ESPWiFi.h>
#include <ESPHTTPClient.h>
#include <ArduinoOTA.h>
#include <JsonListener.h>

#include <time.h>
#include <sys/time.h>
#include <coredecls.h>

#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include "Wire.h"
#include "OpenWeatherMapCurrent.h"
#include "OpenWeatherMapForecast.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

#include "config.h"
#include "dino_game.h"

// Uncomment for serial debug logging
// #define DEBUG

#ifdef DEBUG
  #define LOG(x)   Serial.println(x)
  #define LOGF(x)  Serial.print(x)
#else
  #define LOG(x)
  #define LOGF(x)
#endif

/***************************
 * Settings
 **************************/

// POSIX TZ for Switzerland — auto DST, no firmware update needed
#define TZ_INFO "CET-1CEST,M3.5.0/2,M10.5.0/3"

const int UPDATE_INTERVAL_SECS = 20 * 60;
const int WIFI_TIMEOUT_MS = 20000;

// Display
const int I2C_DISPLAY_ADDRESS = 0x3c;
#if defined(ESP8266)
  #if defined(D3)
const int SDA_PIN = D3;
const int SDC_PIN = D4;
  #else
const int SDA_PIN = 0;
const int SDC_PIN = 2;
  #endif
#else
const int SDA_PIN = 5;
const int SDC_PIN = 4;
#endif

String OPEN_WEATHER_MAP_LANGUAGE = "de";
const uint8_t MAX_FORECASTS = 4;
const boolean IS_METRIC = true;

// Swiss German day/month names
const String WDAY_NAMES[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const String MONTH_NAMES[] = {"Jan", "Feb", "Mär", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};

/***************************
 * End Settings
 **************************/

SSD1306Wire   display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
OLEDDisplayUi ui(&display);

OpenWeatherMapCurrentData currentWeather;
OpenWeatherMapCurrent currentWeatherClient;

OpenWeatherMapForecastData forecasts[MAX_FORECASTS];
OpenWeatherMapForecast forecastClient;

// Rain forecast (next 8 × 3h slots = 24h lookahead)
const uint8_t RAIN_SLOTS = 8;
const float RAIN_THRESHOLD = 1.0;  // mm/3h
OpenWeatherMapForecastData rainForecast[RAIN_SLOTS];
uint8_t rainSlotsFound = 0;
float maxRain = 0;        // peak mm/3h across all slots
float totalRain = 0;      // sum across all slots
bool rainExpected = false;

time_t now;
bool readyForWeatherUpdate = false;
bool wifiConnected = false;
long timeSinceLastWUpdate = 0;

// prototypes
void drawProgress(OLEDDisplay *display, int percentage, String label);
void updateData(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawRainForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);

FrameCallback frames[] = { drawDateTime, drawCurrentWeather, drawForecast, drawRainForecast };
int numberOfFrames = 4;

OverlayCallback overlays[] = { drawHeaderOverlay };
int numberOfOverlays = 1;

void setup() {
  Serial.begin(115200);
  Serial.println();

  display.init();
  display.clear();
  display.display();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

  // WiFi with timeout
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  int counter = 0;
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - wifiStart > WIFI_TIMEOUT_MS) {
      LOG("[wifi] timeout");
      break;
    }
    delay(500);
    LOGF(".");
    display.clear();
    display.drawString(64, 10, "WLAN verbinden...");
    display.drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
    display.drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
    display.drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
    display.display();
    counter++;
  }

  wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (wifiConnected) {
    LOG("[wifi] connected, IP=" + WiFi.localIP().toString());
  } else {
    display.clear();
    display.drawString(64, 20, "Kein WLAN");
    display.display();
    delay(2000);
  }

  // OTA updates (only when WiFi is available)
  if (wifiConnected) {
    ArduinoOTA.setHostname("weatherclock");
    ArduinoOTA.onStart([]() {
      display.clear();
      display.drawString(64, 10, "OTA Update...");
      display.display();
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      display.clear();
      display.drawString(64, 10, "OTA Update...");
      display.drawProgressBar(2, 28, 124, 10, progress / (total / 100));
      display.display();
    });
    ArduinoOTA.onEnd([]() {
      display.clear();
      display.drawString(64, 20, "Neustart...");
      display.display();
    });
    ArduinoOTA.begin();
    LOG("[ota] ready");
  }

  configTzTime(TZ_INFO, "ch.pool.ntp.org", "pool.ntp.org");

  ui.setTargetFPS(30);
  ui.setActiveSymbol(activeSymbole);
  ui.setInactiveSymbol(inactiveSymbole);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, numberOfFrames);
  ui.setOverlays(overlays, numberOfOverlays);
  ui.init();

  if (wifiConnected) {
    updateData(&display);
  }

  gameInit();
}

void loop() {
  ArduinoOTA.handle();

  // Dino game takes over the display when active
  if (gameLoop(&display)) {
    delay(1000 / 20); // ~20 FPS for game
    return;
  }

  // Normal weather station mode
  if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS)) {
    readyForWeatherUpdate = true;
    timeSinceLastWUpdate = millis();
  }

  if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED) {
    updateData(&display);
  }

  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);
  }
}

void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void updateData(OLEDDisplay *display) {
  drawProgress(display, 10, "Zeit aktualisieren...");

  drawProgress(display, 30, "Wetter laden...");
  currentWeatherClient.setMetric(IS_METRIC);
  currentWeatherClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  currentWeatherClient.updateCurrentById(&currentWeather, OWM_API_KEY, OWM_LOCATION_ID);
  LOG("[weather] temp=" + String(currentWeather.temp) + " desc=" + currentWeather.description);

  drawProgress(display, 50, "Vorhersage laden...");
  forecastClient.setMetric(IS_METRIC);
  forecastClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  uint8_t allowedHours[] = {12};
  forecastClient.setAllowedHours(allowedHours, sizeof(allowedHours));
  uint8_t found = forecastClient.updateForecastsById(forecasts, OWM_API_KEY, OWM_LOCATION_ID, MAX_FORECASTS);
  (void)found;
  LOG("[forecast] found=" + String(found) + "/" + String(MAX_FORECASTS));
#ifdef DEBUG
  for (uint8_t i = 0; i < found; i++) {
    LOG("[forecast] [" + String(i) + "] temp=" + String(forecasts[i].temp) + " t=" + String(forecasts[i].observationTime));
  }
#endif

  // Rain forecast (next 24h in 3h slots, no hour filter)
  drawProgress(display, 80, "Regen prüfen...");
  OpenWeatherMapForecast rainClient;
  rainClient.setMetric(IS_METRIC);
  rainClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  rainSlotsFound = rainClient.updateForecastsById(rainForecast, OWM_API_KEY, OWM_LOCATION_ID, RAIN_SLOTS);
  rainExpected = false;
  maxRain = 0;
  totalRain = 0;
  for (uint8_t i = 0; i < rainSlotsFound; i++) {
    LOG("[rain] slot " + String(i) + " rain=" + String(rainForecast[i].rain) + "mm");
    totalRain += rainForecast[i].rain;
    if (rainForecast[i].rain > maxRain) maxRain = rainForecast[i].rain;
    if (rainForecast[i].rain >= RAIN_THRESHOLD) rainExpected = true;
  }

  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Fertig!");
  delay(1000);
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[32];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  sprintf_P(buff, PSTR("%s, %02d.%02d.%04d"), WDAY_NAMES[timeInfo->tm_wday].c_str(), timeInfo->tm_mday, timeInfo->tm_mon+1, timeInfo->tm_year + 1900);
  display->drawString(64 + x, 5 + y, String(buff));

  display->setFont(ArialMT_Plain_24);
  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64 + x, 15 + y, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y, currentWeather.description);

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(60 + x, 5 + y, temp);

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather.iconMeteoCon);
}

void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {
  time_t observationTimestamp = forecasts[dayIndex].observationTime;
  struct tm* timeInfo;
  timeInfo = localtime(&observationTimestamp);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, WDAY_NAMES[timeInfo->tm_wday]);

  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, forecasts[dayIndex].iconMeteoCon);
  String temp = String(forecasts[dayIndex].temp, 0) + (IS_METRIC ? "°C" : "°F");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawRainForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 0 + y, "Regen 24h");

  // Total mm on the right
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 0 + y, String(totalRain, 1) + "mm");

  if (rainSlotsFound == 0) {
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(64 + x, 22 + y, "Keine Daten");
    return;
  }

  // Bar chart area: y=13..42 (30px tall), 8 bars
  const int barAreaTop = 13;
  const int barAreaH = 28;
  const int barW = 12;
  const int gap = 2;
  const int startX = 4;

  // Scale: cap at 10mm for full height, auto-scale if higher
  float scale = maxRain > 10.0 ? maxRain : 10.0;

  for (uint8_t i = 0; i < rainSlotsFound; i++) {
    int bx = startX + i * (barW + gap) + (int)x;
    int barH = (int)(rainForecast[i].rain / scale * barAreaH);
    if (barH < 1 && rainForecast[i].rain > 0) barH = 1;

    // Draw bar
    if (barH > 0) {
      display->fillRect(bx, barAreaTop + barAreaH - barH + (int)y, barW, barH);
    }

    // Draw outline for empty bars
    display->drawRect(bx, barAreaTop + (int)y, barW, barAreaH);

    // Time label below bar
    time_t obsTime = (time_t)rainForecast[i].observationTime;
    struct tm* ti = localtime(&obsTime);
    char lbl[4];
    sprintf_P(lbl, PSTR("%02d"), ti->tm_hour);
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(bx + barW / 2, 42 + (int)y, String(lbl));
  }
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[14];
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);

  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 54, String(buff));
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  String hum = String((int)currentWeather.humidity) + "%";
  display->drawString(64, 54, hum);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(128, 54, temp);
  display->drawHorizontalLine(0, 52, 128);

  // Rain intensity bar (fills along y=51, proportional to maxRain, cap 10mm)
  if (maxRain > 0) {
    int barW = (int)(maxRain / 10.0 * 128);  // 10mm = full width
    if (barW > 128) barW = 128;
    if (barW < 2) barW = 2;
    display->fillRect(0, 51, barW, 2);
  }
}
