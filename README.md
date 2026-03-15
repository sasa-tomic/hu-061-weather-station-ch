# Weatherstation CH

ESP01-based weather station with SSD1306 OLED display, localized for Switzerland.
Based on [ThingPulse Weather Station](https://github.com/ThingPulse/esp8266-weather-station).

## Features

- Current weather + 4-day forecast via OpenWeatherMap
- Swiss German UI (day/month names, DD.MM.YYYY date format)
- Automatic CET/CEST daylight saving time switching
- Bottom bar: time, humidity, temperature
- WiFi timeout with offline fallback

## Setup

```bash
uv run setup.py
```

This will:
- Prompt for WiFi credentials and OpenWeatherMap API key
- Generate `config.h`
- Install PlatformIO (via uv, automatic) and build/upload

Use `uv run setup.py --config` to only generate config.h without building.

Or manually: copy `config.example.h` to `config.h` and fill in your values.

### Get an API key

Free at https://docs.thingpulse.com/how-tos/openweathermap-key/

### Find your location ID

Search at https://old.openweathermap.org/find?q= — the number at the end of the result URL is your location ID.

## Build

### PlatformIO (recommended)

```bash
uv run pio run                  # compile
uv run pio run -t upload        # flash
uv run pio device monitor       # serial monitor
```

### Arduino IDE

1. Board: Generic ESP8266 Module (1M flash)
2. Install libraries: ESP8266 Weather Station, ESP8266 OLED SSD1306, Json Streaming Parser
3. Open `weatherstation-ch.ino`, compile & upload

## Hardware

- ESP-01 (ESP8266, 1MB flash)
- SSD1306 128x64 OLED (I2C, address 0x3c)
- I2C wiring: SDA=GPIO0, SCL=GPIO2

## Debug

Uncomment `#define DEBUG` in the .ino for serial logging at 115200 baud.
