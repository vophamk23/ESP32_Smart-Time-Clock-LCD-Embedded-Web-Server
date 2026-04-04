# 🕐 Smart Clock ESP32

<div align="center">

**Multi-functional Smart Clock System with ESP32**
<br>

![Smart Clock System](report/Smart%20Time%20Clock.jpg)

_Real-time scheduling · Dual displays · NTP sync · Environmental monitoring · Web Dashboard_

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-teal.svg)](https://www.arduino.cc/)
[![Build: PlatformIO](https://img.shields.io/badge/Build-PlatformIO-orange.svg)](https://platformio.org/)

</div>

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Architecture](#-architecture)
- [Hardware](#-hardware)
- [Pin Configuration](#-pin-configuration)
- [Installation](#-installation)
- [Web Dashboard](#-web-dashboard)
- [Usage Guide](#-usage-guide)
- [Serial Commands](#-serial-commands)
- [Display Formats](#-display-formats)
- [Project Structure](#-project-structure)
- [Troubleshooting](#-troubleshooting)

---

## 🌟 Overview

**Smart Clock ESP32** is an embedded system built on the **ESP32 DevKit** platform, implementing a **Cooperative Scheduler** with a 10ms hardware timer tick. The system synchronizes time via **NTP over WiFi** on boot, then stores it in a **DS3231 RTC** module for accurate offline timekeeping.

A built-in **HTTP Web Dashboard** (served from SPIFFS) allows full remote control via any browser — no app required.

### Key Highlights

| Feature                | Details                                   |
| ---------------------- | ----------------------------------------- |
| **Microcontroller**    | ESP32 (Dual-core, 240MHz)                 |
| **Scheduler**          | Cooperative, 10ms tick (Hardware Timer)   |
| **Time Source**        | DS3231 RTC + NTP sync (UTC+7)             |
| **Displays**           | MAX7219 8-digit LED + I2C LCD 16x2        |
| **Sensor**             | DHT11 (Temperature & Humidity)            |
| **Persistent Storage** | ESP32 Flash via Preferences API           |
| **Operating Modes**    | 5 modes via 3 push buttons or Web UI      |
| **Web Dashboard**      | LCD retro UI, served from SPIFFS via HTTP |
| **WiFi**               | AP + STA dual-mode (WIFI_AP_STA)          |

---

## ✨ Features

### 🕐 Mode 1 — Date & Time

- Real-time clock from DS3231 (±2ppm accuracy)
- Displays HH:MM on LED, HH:MM:SS + DD/MM/YYYY on LCD
- Time persists through power cycles via RTC battery

### 🌡️ Mode 2 — Temperature & Humidity

- DHT11 sensor, refreshed every 2 seconds
- Readings cached in global variables — no sensor spam
- Temperature: 0–50°C (±2°C) / Humidity: 20–90% RH (±5%)

### ⏰ Mode 3 — Alarm Clock

- Adjustable hour/minute via SET + INC buttons or Web UI
- **Alarm saved to Flash** — persists after reboot
- Triggers in **any mode**, not just while viewing alarm screen
- Audio (buzzer) + visual (LED blink) alert

### ⏱️ Mode 4 — Stopwatch

- Centisecond precision (0.01s)
- Up to 5 laps, viewable with INC button or Web UI
- Pause / resume support

### ⏲️ Mode 5 — Countdown Timer

- Configurable: 0–99h, 0–59m, 0–59s
- 10-second increment on seconds field for quick setup
- Audio + visual alert on completion

### 🌐 NTP Time Sync

- Connects to WiFi on boot → syncs RTC → stays connected (AP+STA mode)
- Graceful fallback: if no WiFi, RTC holds previous time
- NTP servers: `pool.ntp.org`, `time.google.com`, `time.cloudflare.com`

### 🖥️ Web Dashboard _(New)_

- Retro LCD-style interface served from SPIFFS
- Real-time status polling every 1 second (`/api/status`)
- 60fps client-side interpolation for stopwatch & countdown
- Full control: mode switching, alarm set/stop, stopwatch, countdown
- Runs in **AP+STA dual mode** — accessible from both home network and direct AP connection

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────┐
│                      ESP32 Core 1                       │
│                                                         │
│  setup()                                                │
│    ├── initButtons()    → GPIO config                   │
│    ├── initDisplay()    → MAX7219 init                  │
│    ├── initSensors()    → RTC begin (Wire init)         │
│    ├── initLCD()        → reuse Wire (no double begin)  │
│    ├── initWiFiAndNTP() → sync RTC, keep WiFi ON        │
│    ├── initWebServer()  → SPIFFS + HTTP routes + AP     │
│    └── SCH_Init()       → start scheduler               │
│                                                         │
│  loop()                                                 │
│    ├── SCH_Dispatch_Tasks()                             │
│    └── Serial command handler                           │
│                                                         │
│  Hardware Timer ISR (every 10ms)                        │
│    └── SCH_Update() — O(1)                              │
│                                                         │
│  Task Schedule:                                         │
│    ├── Task_CheckButtons     50ms   Button debounce     │
│    ├── Task_UpdateDisplay   100ms   LED 7-segment       │
│    ├── Task_UpdateLCD       100ms   LCD 16x2            │
│    ├── Task_HandleLEDBlink  200ms   LED indicator       │
│    ├── Task_CheckAlarm     1000ms   Alarm trigger       │
│    ├── Task_ReadSensors    2000ms   DHT11 read          │
│    ├── Task_SerialMonitor  5000ms   Debug output        │
│    └── Task_WebServer_Handler 50ms HTTP poll            │
└─────────────────────────────────────────────────────────┘
```

### Scheduler Design

- **Non-preemptive (Cooperative)** — tasks never interrupt each other
- **O(1) ISR** — only decrements the first task's delay counter
- **O(n) Dispatch** — runs in `loop()`, safe for Serial/I2C/SPI calls
- **Dynamic task management** — add/delete tasks at runtime

### Data Flow

```
DHT11 ──→ Task_ReadSensors (2s) ──→ g_temp / g_humi
                                         │
                              Task_UpdateDisplay (100ms)
                              Task_UpdateLCD    (100ms)
                              Task_SerialMonitor (5s)
                              /api/status  (on demand)

DS3231 ──→ getRTC()->now() ──→ displayDateTime / Task_CheckAlarm

Flash ──→ initSensors() ──→ alarmHour / alarmMinute
             (boot)
handleIncButton() / clockData_setAlarm() ──→ Preferences ──→ Flash

Browser ──→ GET /api/status ──→ clockData_buildStatusJSON()
Browser ──→ POST /api/*     ──→ clock_data functions
```

### Web Architecture (Layered)

```
clock_webserver.cpp   ← HTTP only: parse request, call clock_data, send response
       │
clock_data.cpp        ← Business logic: mode switch, alarm, stopwatch, countdown
       │
global_vars.cpp       ← Shared state (no HTTP dependency)
```

---

## 🔧 Hardware

| Component     | Description                      | Qty |
| ------------- | -------------------------------- | --- |
| ESP32 DevKit  | Main MCU (30-pin)                | 1   |
| MAX7219       | 8-digit 7-segment LED driver     | 1   |
| LCD I2C 16x2  | Alphanumeric display (addr 0x27) | 1   |
| DS3231        | RTC module with CR2032 battery   | 1   |
| DHT11         | Temperature & humidity sensor    | 1   |
| Active Buzzer | 5V, audio alert                  | 1   |
| LED           | Indicator (any color)            | 1   |
| Resistor 220Ω | Current limiting for LED         | 1   |
| Push Button   | Tactile switch                   | 3   |
| Jumper Wires  | Male-to-male / male-to-female    | ~20 |

---

## 📌 Pin Configuration

```
ESP32 DevKit
│
├── SPI (MAX7219)
│     DIN → GPIO 13
│     CLK → GPIO 14
│     CS  → GPIO 12
│
├── I2C shared bus (LCD 16x2 + DS3231)
│     SDA → GPIO 21
│     SCL → GPIO 22
│
├── DHT11
│     DATA → GPIO 27
│
├── Buttons (INPUT_PULLUP — LOW when pressed)
│     MODE → GPIO 16
│     SET  → GPIO 17
│     INC  → GPIO  5
│
└── Outputs
      Buzzer → GPIO 32
      LED    → GPIO 33 → 220Ω → GND
```

> **Important:** `initSensors()` must be called **before** `initLCD()` in `setup()`
> so that `Wire.begin()` is only called once (inside `rtc.begin()`).

---

## 💻 Installation

### 1. Clone the repository

```bash
git clone https://github.com/vophamk23/smart-clock-esp32.git
cd smart-clock-esp32
```

### 2. Configure WiFi credentials

Edit `include/config.h`:

```cpp
#define WIFI_SSID  "your_wifi_ssid"
#define WIFI_PASS  "your_wifi_password"

// Web server config (also in config.h)
#define WEB_USE_AP  1          // 0 = STA only, 1 = AP only, 2 = AP+STA
#define AP_SSID     "SmartClock"
#define AP_PASS     ""         // open AP; set >=8 chars for password
#define WEB_PORT    80
```

### 3. Dependencies (`platformio.ini`)

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    adafruit/RTClib@^2.1.1
    adafruit/DHT sensor library@^1.4.4
    marcoschwartz/LiquidCrystal_I2C@^1.1.4
    wayoda/LedControl@^1.0.6
```

### 4. Build & Upload Firmware

```bash
pio run -t upload        # build + upload firmware
pio device monitor       # open serial monitor (115200 baud)
```

### 5. Upload Web Dashboard (SPIFFS)

Place these 3 files in the `data/` directory:

```
data/
  index.html
  style.css
  app.js
```

Then flash the filesystem:

```bash
pio run -t uploadfs
```

Verify the upload log shows all 3 files with correct names:

```
/app.js
/index.html
/style.css    ← must be style.css, NOT stype.css
```

---

## 🖥️ Web Dashboard

### Accessing the Dashboard

| Network         | How to connect                   | URL                                 |
| --------------- | -------------------------------- | ----------------------------------- |
| Home WiFi (STA) | Device on same network           | `http://<STA_IP>` (shown in Serial) |
| Direct AP       | Connect to WiFi **"SmartClock"** | `http://192.168.4.1`                |

In AP+STA mode (`WEB_USE_AP 2`), both access methods work simultaneously.

### Dashboard Features

- **LCD retro UI** — green-on-black phosphor display aesthetic with scanlines
- **Live clock** — updates every second from `/api/status`
- **Mode buttons** — switch between all 5 modes from the browser
- **Sensor panel** — shows temperature and humidity in real time
- **Alarm panel** — set hour/minute, stop active alarm
- **Stopwatch panel** — Start / Lap+Stop / Reset, displays up to 5 laps
- **Countdown panel** — set HH:MM:SS, Start / Stop / Reset
- **Physical button sync** — if mode is changed via hardware buttons, web UI syncs automatically on next poll

### REST API

All endpoints return JSON. Body format for POST is `application/json`.

| Method | Endpoint               | Body                                       | Description                |
| ------ | ---------------------- | ------------------------------------------ | -------------------------- |
| `GET`  | `/api/status`          | —                                          | Full system state snapshot |
| `POST` | `/api/mode`            | `{"mode": 0–4}`                            | Switch display mode        |
| `POST` | `/api/alarm`           | `{"hour": H, "minute": M}`                 | Set alarm time             |
| `POST` | `/api/alarm/stop`      | —                                          | Silence active alarm       |
| `POST` | `/api/stopwatch/start` | —                                          | Start stopwatch            |
| `POST` | `/api/stopwatch/stop`  | —                                          | Stop + save lap            |
| `POST` | `/api/stopwatch/reset` | —                                          | Reset stopwatch & laps     |
| `POST` | `/api/countdown/set`   | `{"hours": H, "minutes": M, "seconds": S}` | Set countdown duration     |
| `POST` | `/api/countdown/start` | —                                          | Start countdown            |
| `POST` | `/api/countdown/stop`  | —                                          | Pause countdown            |
| `POST` | `/api/countdown/reset` | —                                          | Reset countdown            |

#### Example `/api/status` response

```json
{
  "time": {
    "hour": 14,
    "minute": 23,
    "second": 7,
    "day": 1,
    "month": 4,
    "year": 2026,
    "weekday": "Wed"
  },
  "sensor": { "temp": 30.1, "humidity": 55.0 },
  "mode": 3,
  "modeName": "STOPWATCH",
  "alarm": { "hour": 7, "minute": 0, "triggered": false, "editHour": true },
  "stopwatch": {
    "running": true,
    "elapsed": 83450,
    "lapCount": 1,
    "laps": [41200]
  },
  "countdown": {
    "running": false,
    "editing": true,
    "triggered": false,
    "remaining": 0,
    "duration": 0,
    "editHours": 0,
    "editMinutes": 5,
    "editSeconds": 0
  },
  "system": { "heap": 235672, "uptime": 145 }
}
```

### Web File Structure

```
data/
├── index.html   # HTML structure: LCD bezel, panels, mode buttons, action controls
├── style.css    # Retro LCD theme: VT323 + Share Tech Mono fonts, scanlines, green phosphor
└── app.js       # All logic: API calls, UI panel switching, 60fps render loop
```

> **Note:** `app.js` is a single self-contained file — no build tools required.
> It handles both API communication and UI panel switching to avoid scope conflicts.

---

## 📖 Usage Guide

### Button Controls

| Button   | Mode      | Action                                         |
| -------- | --------- | ---------------------------------------------- |
| **MODE** | Any       | Cycle to next mode                             |
| **SET**  | Alarm     | Toggle edit: Hour ↔ Minute / Stop alarm        |
| **SET**  | Stopwatch | Start → Stop + save lap → Resume               |
| **SET**  | Countdown | Confirm field → Start / Stop alert             |
| **INC**  | Alarm     | Increment selected field (auto-saves to Flash) |
| **INC**  | Stopwatch | View saved laps                                |
| **INC**  | Countdown | Increment field / Reset to 00:00:00            |

### Mode Cycle

```
TEMP/HUMI → DATE/TIME → ALARM → STOPWATCH → COUNTDOWN
   [MODE]      [MODE]    [MODE]    [MODE]       [MODE] → (back)
```

---

## 💻 Serial Commands

Open serial monitor at **115200 baud**:

| Command   | Action                               |
| --------- | ------------------------------------ |
| `m` / `M` | Next mode                            |
| `0`       | Jump to TEMP/HUMI                    |
| `1`       | Jump to DATE/TIME                    |
| `2`       | Jump to ALARM                        |
| `3`       | Jump to STOPWATCH (reset)            |
| `4`       | Jump to COUNTDOWN (edit mode)        |
| `r` / `R` | Reset alarm to default + clear Flash |
| `?`       | Show help                            |

### Serial Monitor Output (every 5s)

```
========== SMART CLOCK ==========
  Time : 16:07:06
  Date : 01/04/2026
  Mode : STOPWATCH
---------------------------------
  State: RUNNING
  Time : 00h 02m 35s
  Laps : 2/5
    Lap1: 01m12s04
    Lap2: 01m23s08
---------------------------------
  Temp : 29.6C  Humi: 54%
  Alarm: 07:00 [OFF]
  SW   : RUN  CD: STOP
  Heap : 283848 bytes
=================================
```

---

## 📺 Display Formats

### LED 7-Segment (8 digits)

```
Position: [7][6].[5][4]   [3][2].[1][0]
           Left Group       Right Group
```

| Mode      | Left (digits 7–4) | Right (digits 3–0) |
| --------- | ----------------- | ------------------ |
| Temp/Humi | `TT.T C`          | `HH.H H`           |
| Date/Time | `HH.MM` (time)    | `DD.MM` (date)     |
| Alarm     | `HH.MM` (now)     | `HH.MM` (alarm)    |
| Stopwatch | `HH.MM`           | `SS.CC`            |
| Countdown | `HH.MM`           | `SS.CC`            |

### LCD 16x2

```
Mode 1 — Temp/Humi:          Mode 2 — Date/Time:
┌────────────────┐           ┌────────────────┐
│TEMP: 29.6 C    │           │Time: 14:35:42  │
│HUMI: 54%       │           │Date: 01/04/2026│
└────────────────┘           └────────────────┘

Mode 3 — Alarm:              Mode 3 — Triggered:
┌────────────────┐           ┌────────────────┐
│Now:  14:35:42  │           │ALARM RINGING!  │
│Alarm:    07:00 │           │Press SET stop  │
└────────────────┘           └────────────────┘

Mode 4 — Stopwatch:          Mode 5 — Countdown:
┌────────────────┐           ┌────────────────┐
│Stopwatch  RUN  │           │Countdown  RUN  │
│01:23:45.67 L2  │           │00:02:45.00     │
└────────────────┘           └────────────────┘
```

---

## 📁 Project Structure

```
smart-clock-esp32/
├── src/
│   ├── main.cpp                 # Entry point, setup & loop
│   ├── App_Tasks.cpp/h          # All scheduler task implementations
│   ├── scheduler.cpp/h          # Cooperative scheduler (HW Timer)
│   ├── sensor_handler.cpp/h     # DHT11 + DS3231 init, Flash load
│   ├── led_7seg_display.cpp/h   # MAX7219 display functions
│   ├── lcd_display.cpp/h        # LCD 16x2 functions
│   ├── button_handler.cpp/h     # Debounce + button logic + Flash save
│   ├── wifi_clock.cpp/h         # WiFi connect + NTP sync (stays ON)
│   ├── ntp_sync.cpp/h           # NTP query → DS3231 write
│   ├── clock_webserver.cpp/h    # HTTP layer: routes → clock_data calls
│   ├── clock_data.cpp/h         # Business logic: mode/alarm/SW/CD control
│   ├── global_vars.cpp/h        # Shared state (g_temp, g_humi, etc.)
│   └── config.h                 # Pin definitions + timing + web config
├── data/                        # SPIFFS filesystem (upload separately)
│   ├── index.html               # Web dashboard HTML
│   ├── style.css                # Retro LCD theme
│   └── app.js                   # Dashboard logic + API client
├── platformio.ini
├── LICENSE
└── README.md
```

---

## 🐛 Troubleshooting

| Problem                               | Likely Cause              | Fix                                                       |
| ------------------------------------- | ------------------------- | --------------------------------------------------------- |
| LCD shows nothing                     | Wrong I2C address         | Try `0x27` or `0x3F` in `lcd_display.cpp`                 |
| LCD shows black squares               | Contrast too high         | Adjust potentiometer on LCD back                          |
| DHT reads `NaN`                       | Bad connection            | Check 3.3V power + GPIO 27 wiring                         |
| RTC loses time                        | Dead battery              | Replace CR2032 in DS3231 module                           |
| NTP sync fails                        | Wrong credentials         | Check `WIFI_SSID`/`WIFI_PASS` in `config.h`               |
| Alarm resets on reboot                | Flash not saved           | Ensure `Preferences` write in `clockData_setAlarm()`      |
| `Wire already started` warning        | Wrong init order          | Call `initSensors()` **before** `initLCD()`               |
| Buzzer stuck ON                       | Missing LOW init          | Check `digitalWrite(BUZZER_PIN, LOW)` in `initButtons()`  |
| Alarm doesn't trigger                 | Old mode-only check       | `Task_CheckAlarm` must not check `displayMode`            |
| Web page has no CSS                   | Wrong filename in `data/` | Must be `style.css`, not `stype.css`                      |
| Web page shows raw HTML               | SPIFFS not flashed        | Run `pio run -t uploadfs` after uploading firmware        |
| Web inputs reset while typing         | Old `app.js` version      | Update to latest `app.js` (uses `activeElement` check)    |
| Web doesn't reach ESP32 (STA)         | Different subnet          | Check Serial for STA IP; ensure same WiFi network         |
| Web doesn't update after button press | Scope conflict            | Ensure single `app.js` file, no inline `<script>` in HTML |
| Countdown/Stopwatch not updating      | Old dual-JS architecture  | Replace both `index.html` and `app.js` together           |

---

## 📄 **License**

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2024 Phạm Công Võ

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

[Full license text...]
```

---

## 👨‍💻 **Author & Contact**

**Phạm Công Võ**

- 📧 Email: [congvolv1@gmail.com](mailto:congvolv1@gmail.com)
- 🐙 GitHub: [@vophamk23](https://github.com/vophamk23)

---

## 🙏 **Acknowledgments**

- **ESP32 Community** for excellent documentation
- **Adafruit** for RTClib and DHT libraries
- **Arduino** for the accessible development platform
- **Contributors** who helped improve this project
