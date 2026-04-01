# 🕐 Smart Clock ESP32

<div align="center">


**Multi-functional Smart Clock System with ESP32**
<br>

![Smart Clock System](Smart%20Time%20Clock.jpg)

*Real-time scheduling · Dual displays · NTP sync · Environmental monitoring*

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
- [Usage Guide](#-usage-guide)
- [Serial Commands](#-serial-commands)
- [Display Formats](#-display-formats)
- [Project Structure](#-project-structure)
- [Troubleshooting](#-troubleshooting)

---

## 🌟 Overview

**Smart Clock ESP32** is an embedded system built on the **ESP32 DevKit** platform, implementing a **Cooperative Scheduler** with a 10ms hardware timer tick. The system synchronizes time via **NTP over WiFi** on boot, then stores it in a **DS3231 RTC** module for accurate offline timekeeping.

### Key Highlights

| Feature | Details |
|---------|---------|
| **Microcontroller** | ESP32 (Dual-core, 240MHz) |
| **Scheduler** | Cooperative, 10ms tick (Hardware Timer) |
| **Time Source** | DS3231 RTC + NTP sync (UTC+7) |
| **Displays** | MAX7219 8-digit LED + I2C LCD 16x2 |
| **Sensor** | DHT11 (Temperature & Humidity) |
| **Persistent Storage** | ESP32 Flash via Preferences API |
| **Operating Modes** | 5 modes via 3 push buttons |

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
- Adjustable hour/minute via SET + INC buttons
- **Alarm saved to Flash** — persists after reboot
- Triggers in **any mode**, not just while viewing alarm screen
- Audio (buzzer) + visual (LED blink) alert

### ⏱️ Mode 4 — Stopwatch
- Centisecond precision (0.01s)
- Up to 5 laps, viewable with INC button
- Pause / resume support

### ⏲️ Mode 5 — Countdown Timer
- Configurable: 0–99h, 0–59m, 0–59s
- 10-second increment on seconds field for quick setup
- Audio + visual alert on completion

### 🌐 NTP Time Sync
- Connects to WiFi on boot → syncs RTC → disconnects
- Frees ~40KB RAM after sync (WiFi OFF)
- Graceful fallback: if no WiFi, RTC holds previous time
- NTP servers: `pool.ntp.org`, `time.google.com`, `time.cloudflare.com`

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────┐
│                      ESP32 Core 1                       │
│                                                         │
│  setup()                                                │
│    ├── initSensors()    → RTC begin (Wire init)         │
│    ├── initLCD()        → reuse Wire (no double begin)  │
│    ├── initWiFiAndNTP() → sync RTC → WiFi OFF           │
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
│    └── Task_SerialMonitor  5000ms   Debug output        │
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

DS3231 ──→ getRTC()->now() ──→ displayDateTime / Task_CheckAlarm

Flash ──→ initSensors() ──→ alarmHour / alarmMinute
             (boot)
handleIncButton() ──→ saveAlarm() ──→ Flash
```

---

## 🔧 Hardware

| Component | Description | Qty |
|-----------|-------------|-----|
| ESP32 DevKit | Main MCU (30-pin) | 1 |
| MAX7219 | 8-digit 7-segment LED driver | 1 |
| LCD I2C 16x2 | Alphanumeric display (addr 0x27) | 1 |
| DS3231 | RTC module with CR2032 battery | 1 |
| DHT11 | Temperature & humidity sensor | 1 |
| Active Buzzer | 5V, audio alert | 1 |
| LED | Indicator (any color) | 1 |
| Resistor 220Ω | Current limiting for LED | 1 |
| Push Button | Tactile switch | 3 |
| Jumper Wires | Male-to-male / male-to-female | ~20 |

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
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASS "your_wifi_password"
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

### 4. Build & Upload

```bash
pio run -t upload        # build + upload firmware
pio device monitor       # open serial monitor (115200 baud)
```

---

## 📖 Usage Guide

### Button Controls

| Button | Mode | Action |
|--------|------|--------|
| **MODE** | Any | Cycle to next mode |
| **SET** | Alarm | Toggle edit: Hour ↔ Minute / Stop alarm |
| **SET** | Stopwatch | Start → Stop + save lap → Resume |
| **SET** | Countdown | Confirm field → Start / Stop alert |
| **INC** | Alarm | Increment selected field (auto-saves) |
| **INC** | Stopwatch | View saved laps |
| **INC** | Countdown | Increment field / Reset to 00:00:00 |

### Mode Cycle

```
TEMP/HUMI → DATE/TIME → ALARM → STOPWATCH → COUNTDOWN
   [MODE]      [MODE]    [MODE]    [MODE]       [MODE] → (back)
```

---

## 💻 Serial Commands

Open serial monitor at **115200 baud**:

| Command | Action |
|---------|--------|
| `m` / `M` | Next mode |
| `0` | Jump to TEMP/HUMI |
| `1` | Jump to DATE/TIME |
| `2` | Jump to ALARM |
| `3` | Jump to STOPWATCH (reset) |
| `4` | Jump to COUNTDOWN (edit mode) |
| `r` / `R` | Reset alarm to default + clear Flash |
| `?` | Show help |

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

| Mode | Left (digits 7–4) | Right (digits 3–0) |
|------|-------------------|-------------------|
| Temp/Humi | `TT.T C` | `HH.H H` |
| Date/Time | `HH.MM` (time) | `DD.MM` (date) |
| Alarm | `HH.MM` (now) | `HH.MM` (alarm) |
| Stopwatch | `HH.MM` | `SS.CC` |
| Countdown | `HH.MM` | `SS.CC` |

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
│   ├── wifi_clock.cpp/h         # WiFi connect + NTP sync + disconnect
│   ├── ntp_sync.cpp/h           # NTP query → DS3231 write
│   ├── global_vars.cpp/h        # Shared state (g_temp, g_humi, etc.)
│   └── config.h                 # Pin definitions + timing constants
├── platformio.ini
├── LICENSE
└── README.md
```

---

## 🐛 Troubleshooting

| Problem | Likely Cause | Fix |
|---------|-------------|-----|
| LCD shows nothing | Wrong I2C address | Try `0x27` or `0x3F` in `lcd_display.cpp` |
| LCD shows black squares | Contrast too high | Adjust potentiometer on LCD back |
| DHT reads `NaN` | Bad connection | Check 3.3V power + GPIO 27 wiring |
| RTC loses time | Dead battery | Replace CR2032 in DS3231 module |
| NTP sync fails | Wrong credentials | Check `WIFI_SSID`/`WIFI_PASS` in `config.h` |
| Alarm resets on reboot | Flash not saved | Ensure `saveAlarm()` in `handleIncButton()` |
| `Wire already started` warning | Wrong init order | Call `initSensors()` **before** `initLCD()` |
| Buzzer stuck ON | Missing LOW init | Check `digitalWrite(BUZZER_PIN, LOW)` in `initButtons()` |
| Alarm doesn't trigger | Old mode-only check | `Task_CheckAlarm` must not check `displayMode` |

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

---













# 🕐 Smart Clock ESP32

### **Multi-functional Smart Clock System with ESP32**



---

## 📋 **Table of Contents**

- [Overview](#-overview)
- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Pin Configuration](#-pin-configuration)
- [Installation](#-installation)
- [Usage Guide](#-usage-guide)
- [Display Formats](#-display-formats)
- [Project Structure](#-project-structure)
- [Troubleshooting](#-troubleshooting)
- [Future Enhancements](#-future-enhancements)
- [Contributing](#-contributing)
- [License](#-license)

---

## 🌟 **Overview**

**Smart Clock ESP32** is a sophisticated embedded system built on the **ESP32 DevKit** platform, featuring a **Cooperative Scheduler** architecture for efficient multitasking. The system provides real-time information display through dual output interfaces: **MAX7219 LED 7-Segment** and **I2C LCD 16x2**.

### **Key Highlights:**

- ⏰ **5 Operating Modes**: Date-Time, Temperature-Humidity, Alarm, Stopwatch, Countdown
- 🔄 **Real-Time Scheduler**: 10ms tick with O(1) time complexity
- 📊 **Dual Display System**: MAX7219 8-digit LED + 16x2 LCD
- 🌡️ **Environmental Monitoring**: DHT11 sensor integration
- 🔔 **Smart Alerts**: Buzzer + LED indicator
- 💾 **Persistent Timekeeping**: DS3231 RTC module

### **Technical Specifications:**

| Specification | Details |
|--------------|---------|
| **Microcontroller** | ESP32 (Dual-core, 240MHz) |
| **Scheduler Tick** | 10ms (Hardware Timer) |
| **Task Execution** | Cooperative (Non-preemptive) |
| **Display Update** | 100ms refresh rate |
| **Sensor Reading** | 2-second intervals |
| **Button Debounce** | 200ms |

---

## ✨ **Features**

### 🕰️ **Mode 1: Date & Time**
- Synchronized with **DS3231 RTC** (±2ppm accuracy)
- Displays hours, minutes, seconds
- Full date display: DD/MM/YYYY
- No time loss on power cycle

### 🌡️ **Mode 2: Temperature & Humidity**
- Real-time DHT11 sensor readings
- Temperature range: 0-50°C (±2°C accuracy)
- Humidity range: 20-90% RH (±5% accuracy)
- Auto-refresh every 2 seconds

### ⏰ **Mode 3: Alarm Clock**
- Adjustable hour/minute settings
- Visual feedback: Blinking display during edit
- Audio-visual alert: Buzzer + LED
- Quick alarm disable with SET button

### ⏱️ **Mode 4: Stopwatch**
- Precision: Centiseconds (0.01s)
- Lap recording: Up to 5 laps
- Pause/resume capability
- Lap review function

### ⏲️ **Mode 5: Countdown Timer**
- Adjustable: Hours (0-99), Minutes (0-59), Seconds (0-59)
- 10-second increment for quick setup
- Time-up alert: Buzzer + LED
- Quick reset to 00:00:00

---

## 🔧 **Hardware Requirements**

| Component | Description | Quantity | Notes |
|-----------|-------------|----------|-------|
| **ESP32 DevKit** | Main microcontroller | 1 | 30-pin version recommended |
| **MAX7219** | 8-digit 7-segment LED driver | 1 | Common cathode display |
| **LCD I2C 16x2** | Alphanumeric display | 1 | I2C address: 0x27 or 0x3F |
| **DS3231** | Real-Time Clock module | 1 | Includes CR2032 battery |
| **DHT11** | Temperature & humidity sensor | 1 | 3.3V compatible |
| **Buzzer** | Active buzzer | 1 | 5V recommended |
| **LED** | Indicator LED (any color) | 1 | - |
| **Resistor** | 220Ω for LED | 1 | - |
| **Push Buttons** | Tactile switches | 3 | MODE, SET, INC |
| **Resistors** | 10kΩ pull-up (if needed) | 3 | For buttons |
| **Breadboard** | Prototyping board | 1 | Or custom PCB |
| **Jumper Wires** | Male-to-male/female | ~20 | - |
| **Power Supply** | 5V adapter or USB | 1 | Min 1A recommended |

### **Optional Components:**
- Enclosure/case for finished product
- PCB for permanent installation
- Heat sink for ESP32 (if running hot)

---

## 📌 **Pin Configuration**

```
┌─────────────────────────────────────────────────────┐
│                    ESP32 DevKit                     │
├─────────────────────────────────────────────────────┤
│                                                     │
│  MAX7219 LED 7-Segment:                             │
│    ├── DIN  → GPIO 13                               │
│    ├── CLK  → GPIO 14                               │
│    └── CS   → GPIO 12                               │
│                                                     │
│  LCD I2C 16x2:                                      │
│    ├── SDA  → GPIO 21                               │
│    └── SCL  → GPIO 22                               │
│                                                     │
│  DHT11 Sensor:                                      │
│    └── DATA → GPIO 27                               │
│                                                     │
│  Control Buttons:                                   │
│    ├── MODE → GPIO 16 (INPUT_PULLUP)                │
│    ├── SET  → GPIO 17 (INPUT_PULLUP)                │
│    └── INC  → GPIO 5  (INPUT_PULLUP)                │
│                                                     │
│  Audio/Visual Indicators:                           │
│    ├── Buzzer → GPIO 32                             │
│    └── LED    → GPIO 33 → 220Ω → GND                │
│                                                     │
│  Power:                                             │
│    ├── VIN  → 5V                                    │
│    ├── 3V3  → 3.3V output (for sensors)             │
│    └── GND  → Common ground                         │
│                                                     │
└─────────────────────────────────────────────────────┘
```

### **Wiring Diagram:**

```
     MAX7219              ESP32              LCD I2C
   ┌─────────┐         ┌─────────┐         ┌─────────┐
   │  VCC ───┼────────→│   5V    │←────────┼─── VCC  │
   │  GND ───┼────────→│   GND   │←────────┼─── GND  │
   │  DIN ───┼────────→│  GPIO13 │         │         │
   │  CLK ───┼────────→│  GPIO14 │         │         │
   │  CS  ───┼────────→│  GPIO12 │         │         │
   └─────────┘         │  GPIO21 │←────────┼─── SDA  │
                       │  GPIO22 │←────────┼─── SCL  │
                       └─────────┘         └─────────┘

      DHT11                          Buttons
   ┌─────────┐                   ┌──────────────┐
   │  VCC ───┼──→ 3.3V           │ MODE → GPIO16│
   │  DATA ──┼──→ GPIO27         │ SET  → GPIO17│
   │  GND ───┼──→ GND            │ INC  → GPIO5 │
   └─────────┘                   └──────────────┘
```

---

## 💻 **Installation**

### **1. Setup Development Environment**

#### **Option A: Arduino IDE**
1. Download and install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - Go to **File → Preferences**
   - Add to "Additional Board Manager URLs":
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Go to **Tools → Board → Board Manager**
   - Search for "ESP32" and install

#### **Option B: PlatformIO**
1. Install [VS Code](https://code.visualstudio.com/)
2. Install PlatformIO extension
3. Create new project with ESP32 board

### **2. Install Required Libraries**

Open **Library Manager** (Arduino IDE) or edit `platformio.ini` (PlatformIO):

```ini
# Required Libraries
- LedControl by Eberhard Fahle (v1.0.6+)
- RTClib by Adafruit (v2.1.1+)
- DHT sensor library by Adafruit (v1.4.4+)
- LiquidCrystal_I2C by Frank de Brabander (v1.1.2+)
```


**PlatformIO:**
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

### **4. Configure Settings (Optional)**

Edit `config.h` to customize pin assignments or timing:

```cpp
// Pin definitions
#define BTN_MODE_PIN 16
#define BTN_SET_PIN 17
#define BTN_INC_PIN 5

// Timing settings
#define BUTTON_DEBOUNCE 200
#define DISPLAY_INTENSITY 8  // 0-15
```

## 📖 **Usage Guide**

### **Button Controls**

| Button | Function | Description |
|--------|----------|-------------|
| **MODE** | Mode Switch | Cycle through 5 modes (Temp → Clock → Alarm → Stopwatch → Countdown) |
| **SET** | Edit/Confirm | Enter edit mode / Start-Stop / Confirm changes |
| **INC** | Increment | Increase values / View laps / Reset timer |

### **Operating Modes**

#### **🌡️ Mode 1: Temperature & Humidity**
```
Action:        Display:
───────────────────────────────────────────────
Power on   →   25.3°C  67.2%
Wait       →   Auto-updates every 2s
MODE       →   Switch to Mode 2
```

#### **🕐 Mode 2: Date & Time**
```
Action:        LED Display:       LCD Display:
──────────────────────────────────────────────────────
MODE       →   14.35  25.12   →  Time: 14:35:42
                                  Date: 25/12/2024
```

#### **⏰ Mode 3: Alarm**
```
Action:        Result:
───────────────────────────────────────────────
MODE       →   Enter alarm mode
SET        →   Toggle Hour/Minute edit (blinks)
INC        →   Increase selected field
SET        →   Confirm and wait for alarm
[Alarm]    →   Buzzer beeps + LED blinks
SET        →   Stop alarm
```

#### **⏱️ Mode 4: Stopwatch**
```
Action:        Result:
───────────────────────────────────────────────
MODE       →   Enter stopwatch mode (00:00:00.00)
SET        →   Start counting
SET        →   Stop and save lap
SET        →   Resume (save lap on next stop)
INC        →   View saved laps
```

#### **⏲️ Mode 5: Countdown**
```
Action:        Result:
───────────────────────────────────────────────
MODE       →   Enter countdown edit (00:00:00)
SET        →   Select field (Hour → Minute → Second)
INC        →   Increase value (+1h, +1m, +10s)
SET        →   Start countdown
[Time up]  →   Buzzer + LED alert
SET        →   Stop alert, return to edit
INC        →   Quick reset to 00:00:00
```

### **Quick Reference Card**

```
╔══════════════════════════════════════════════════════╗
║              SMART CLOCK - QUICK GUIDE               ║
╠══════════════════════════════════════════════════════╣
║ MODE Button: Switch between 5 modes                 ║
║ SET Button:  Edit / Start / Stop / Confirm          ║
║ INC Button:  +Value / View Laps / Reset             ║
╠══════════════════════════════════════════════════════╣
║ Mode 1: 🌡️  Temperature & Humidity (auto-refresh)   ║
║ Mode 2: 🕐  Current Time & Date                     ║
║ Mode 3: ⏰  Alarm Clock (SET to snooze)             ║
║ Mode 4: ⏱️  Stopwatch (max 5 laps)                  ║
║ Mode 5: ⏲️  Countdown Timer (INC to reset)          ║
╚══════════════════════════════════════════════════════╝
```

### **Task Scheduling Table**

| Task ID | Task Name | Period | Priority | Description |
|---------|-----------|--------|----------|-------------|
| 1 | `Task_CheckButtons` | 50ms | High | Scan 3 buttons with debouncing |
| 2 | `Task_UpdateDisplay` | 100ms | High | Update LED 7-segment display |
| 3 | `Task_UpdateLCD` | 100ms | High | Update LCD 16x2 display |
| 4 | `Task_HandleLEDBlink` | 200ms | Medium | Toggle LED indicator |
| 5 | `Task_CheckAlarm` | 1000ms | Low | Check alarm trigger conditions |
| 6 | `Task_ReadSensors` | 2000ms | Low | Read DHT11 sensor data |

### **Scheduler Characteristics**

- **Tick Resolution:** 10ms (100Hz)
- **Time Complexity:** O(1) for ISR, O(n) for dispatch
- **Task Management:** Dynamic add/delete supported
- **Execution Model:** Non-preemptive (cooperative)
- **Maximum Tasks:** Configurable (default: 10)

---

## 📺 **Display Formats**

### **LED 7-Segment (8 Digits)**

```
Position:  [7][6].[5][4]  [3][2].[1][0]
           ─────────────  ─────────────
            Left Group     Right Group
```

| Mode | Left Group | Right Group | Example |
|------|------------|-------------|---------|
| **Temp/Humidity** | `TT.T°C` | `HH.H%` | `25.3 C` `67.2 H` |
| **Date/Time** | `HH.MM` | `DD.MM` | `14.35` `25.12` |
| **Alarm** | `HH.MM` (now) | `HH.MM` (alarm) | `14.35` `10.30` |
| **Stopwatch** | `HH.MM` | `SS.CC` | `01.23` `45.67` |
| **Countdown** | `HH.MM` | `SS.CC` | `00.03` `00.00` |

### **LCD 16x2 Display**

#### **Mode 1: Temperature & Humidity**
```
┌────────────────┐
│TEMP: 25.3°C    │  ← Line 1: Temperature
│HUMI: 67.2%     │  ← Line 2: Humidity
└────────────────┘
     Column: 0123456789012345
```

#### **Mode 2: Date & Time**
```
┌────────────────┐
│Time: 14:35:42  │  ← Line 1: HH:MM:SS
│Date: 25/12/2024│  ← Line 2: DD/MM/YYYY
└────────────────┘
```

#### **Mode 3: Alarm**
```
┌────────────────┐
│Now: 14:35:42   │  ← Line 1: Current time
│Alarm: 10:30    │  ← Line 2: Alarm time (blinks when editing)
└────────────────┘

When triggered:
┌────────────────┐
│ALARM RINGING!  │
│                │
└────────────────┘
```

#### **Mode 4: Stopwatch**
```
┌────────────────┐
│Stopwatch RUN   │  ← Status: RUN/STOP
│01:23:45.67 L3  │  ← Time + Lap count
└────────────────┘
     └─────────┘└── L3 = 3 laps saved

Viewing laps:
┌────────────────┐
│Lap 1 of 3      │
│00:15:32.45     │
└────────────────┘
```

#### **Mode 5: Countdown**
```
Edit mode:
┌────────────────┐
│Countdown EDIT  │
│00:03:30.00     │  ← Blinks on selected field
└────────────────┘

Running:
┌────────────────┐
│Countdown RUN   │
│00:03:00.00     │  ← Counts down
└────────────────┘

Time up:
┌────────────────┐
│TIME'S UP!      │
│00:00:00.00     │
└────────────────┘
```

---

## 📁 **Project Structure**

```
smart-clock-esp32/
│
├── src/
│   ├── main.cpp                    # Entry point, setup & loop
│   │
│   ├── scheduler/
│   │   ├── scheduler.h             # Scheduler interface
│   │   └── scheduler.cpp           # Timer ISR & task management
│   │
│   ├── tasks/
│   │   ├── App_Tasks.h             # Task declarations
│   │   └── App_Tasks.cpp           # Task implementations
│   │
│   ├── handlers/
│   │   ├── button_handler.h        # Button control interface
│   │   ├── button_handler.cpp      # Debouncing & button logic
│   │   ├── sensor_handler.h        # Sensor interface
│   │   └── sensor_handler.cpp      # DHT11 & RTC initialization
│   │
│   ├── display/
│   │   ├── led_7seg_display.h      # MAX7219 interface
│   │   ├── led_7seg_display.cpp    # LED display functions
│   │   ├── lcd_display.h           # LCD interface
│   │   └── lcd_display.cpp         # LCD display functions
│   │
│   ├── core/
│   │   ├── config.h                # Pin definitions & constants
│   │   ├── global_vars.h           # Global variable declarations
│   │   └── global_vars.cpp         # Global variable definitions
│   │
│   └── utils/
│       └── (future: helpers, formatters)
│
├── docs/
│   ├── WIRING_DIAGRAM.pdf          # Detailed wiring schematic
│   ├── USER_MANUAL.md              # Complete user guide
│   └── API_REFERENCE.md            # Function documentation
│
├── examples/
│   ├── basic_clock/                # Simple clock example
│   ├── alarm_only/                 # Standalone alarm
│   └── sensor_display/             # Temperature display only
│
├── tests/
│   ├── test_scheduler.cpp          # Scheduler unit tests
│   ├── test_buttons.cpp            # Button handler tests
│   └── test_display.cpp            # Display output tests
│
├── hardware/
│   ├── fritzing/                   # Fritzing project files
│   ├── pcb/                        # KiCad PCB design (optional)
│   └── enclosure/                  # 3D printable case files
│
├── .gitignore
├── platformio.ini                  # PlatformIO configuration
├── LICENSE                         # MIT License
└── README.md                       # This file
```

## 🐛 **Troubleshooting**

### **Common Issues**

| Problem | Possible Cause | Solution |
|---------|----------------|----------|
| **LCD shows nothing** | Wrong I2C address | Try `0x27` or `0x3F` in `lcd_display.cpp` |
| **LCD shows squares** | Contrast too high | Adjust potentiometer on LCD module |
| **LED not displaying** | Wrong wiring | Check DIN→13, CLK→14, CS→12 |
| **DHT reads NaN** | Sensor not connected | Verify 3.3V power, data on GPIO27 |
| **RTC loses time** | Dead battery | Replace CR2032 battery in DS3231 |
| **Buttons not working** | No debouncing | Check `BUTTON_DEBOUNCE` value (200ms) |
| **ESP32 crashes** | Stack overflow | Reduce task complexity or increase stack |
| **Flickering display** | Low refresh rate | Verify 100ms task period |
| **Alarm doesn't trigger** | Wrong time set | Check RTC time with Serial Monitor |
| **Buzzer always on** | Stuck HIGH | Check `BUZZER_PIN` initialization |
---

## 🚀 **Future Enhancements**

### **Planned Features**

- [ ] **WiFi Connectivity**
  - NTP time synchronization
  - Web interface for remote control
  - MQTT integration for IoT

- [ ] **Enhanced Alarms**
  - Multiple alarm slots (up to 5)
  - Weekday selection
  - Gradual volume increase
  - Snooze function (5-minute interval)

- [ ] **Data Logging**
  - SD card integration
  - Temperature/humidity history
  - Export to CSV format

- [ ] **Advanced Display**
  - OLED display option (128x64)
  - Color-coded warnings
  - Graph visualization

- [ ] **Mobile App**
  - Bluetooth LE control
  - Android/iOS companion app
  - Push notifications

- [ ] **Power Management**
  - Sleep mode for energy saving
  - Battery backup support
  - Auto-dimming at night

---

## 🤝 **Contributing**

We welcome contributions! Here's how:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/AmazingFeature`)
3. **Commit** your changes (`git commit -m 'Add some AmazingFeature'`)
4. **Push** to the branch (`git push origin feature/AmazingFeature`)
5. **Open** a Pull Request

Please ensure:
- Code follows existing style (comments in English)
- All tests pass
- Update documentation as needed

---



