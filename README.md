# 🕐 Smart Clock ESP32

### **Multi-functional Smart Clock System with ESP32**

<div align="center">

**A feature-rich embedded system with real-time scheduling, dual displays, and environmental monitoring**

<br>


![Smart Clock System](Smart%20Time%20Clock.jpg)



*ESP32-based smart clock featuring LED 7-segment display, LCD screen, RTC module, and temperature sensor*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-teal.svg)](https://www.arduino.cc/)

</div>

---

## 📋 **Table of Contents**

- [Overview](#-overview)
- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Pin Configuration](#-pin-configuration)
- [Installation](#-installation)
- [Usage Guide](#-usage-guide)
- [System Architecture](#%EF%B8%8F-system-architecture)
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

---

