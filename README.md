# 🕐 Smart Clock ESP32

### **Hệ thống đồng hồ thông minh đa chức năng sử dụng ESP32**

<div align="center">
A feature-rich smart clock built with ESP32, featuring real-time display, temperature monitoring, alarm, stopwatch, and countdown timer.
<br>
<img src="" alt="Smart Clock Image"/>
</div>

---

## 📋 **Mục lục**

* [Giới thiệu](#giới-thiệu)
* [Tính năng](#tính-năng)
* [Yêu cầu phần cứng](#yêu-cầu-phần-cứng)
* [Sơ đồ chân](#sơ-đồ-chân)
* [Cài đặt](#cài-đặt)
* [Hướng dẫn sử dụng](#hướng-dẫn-sử-dụng)
* [Kiến trúc hệ thống](#kiến-trúc-hệ-thống)
* [Định dạng hiển thị](#định-dạng-hiển-thị)
* [Cấu trúc dự án](#cấu-trúc-dự-án)
* [Gỡ lỗi](#gỡ-lỗi)
* [Hướng phát triển](#hướng-phát-triển)
* [Tác giả & License](#tác-giả--license)

---

## 🌟 **Giới thiệu**

**Smart Clock ESP32** là một hệ thống nhúng tích hợp nhiều chức năng thời gian thực, được xây dựng trên nền tảng **ESP32 DevKit**. Hệ thống sử dụng kiến trúc **Cooperative Scheduler** để xử lý đa nhiệm, đồng thời hiển thị thông tin trên **LED 7-Segment MAX7219** và **LCD I2C 16x2**.

**Điểm nổi bật:**

* ⏰ **5 chế độ hoạt động**: Thời gian–Ngày tháng, Nhiệt độ–Độ ẩm, Báo thức, Bấm giờ, Đếm ngược.
* 🔄 **Bộ lập lịch thời gian thực** (10ms tick – O(1) update).
* 📊 **Hệ thống hiển thị kép**: MAX7219 + LCD 16x2.
* 🌡️ **Giám sát môi trường**: Cảm biến DHT11.
* 🔔 **Cảnh báo thông minh**: Buzzer + LED.

---

## ✨ **Tính năng**

### 🕰️ Chế độ 1: Ngày & Giờ

* Đồng bộ thời gian với **RTC DS3231**.
* Hiển thị HH:MM:SS và DD/MM/YYYY.

### 🌡️ Chế độ 2: Nhiệt độ & Độ ẩm

* Đọc cảm biến DHT11.
* Cập nhật mỗi 2 giây.

### ⏰ Chế độ 3: Báo thức

* Chỉnh giờ/phút.
* Báo bằng LED + Buzzer.
* Nhấp nháy khi chỉnh.

### ⏱️ Chế độ 4: Đồng hồ bấm giờ

* Độ chính xác centiseconds.
* Ghi tối đa 5 vòng (Lap).

### ⏲️ Chế độ 5: Đếm ngược

* Cài đặt giờ–phút–giây.
* Reset nhanh / cảnh báo khi hết thời gian.

---

## 🔧 **Yêu cầu phần cứng**

| Thành phần            | Mô tả               | Số lượng |
| --------------------- | ------------------- | -------- |
| ESP32 DevKit          | Vi điều khiển chính | 1        |
| MAX7219               | LED 7-seg 8 digit   | 1        |
| LCD I2C 16x2          | Hiển thị phụ        | 1        |
| DS3231                | Mô-đun RTC          | 1        |
| DHT11                 | Cảm biến            | 1        |
| Buzzer                | Còi cảnh báo        | 1        |
| LED + điện trở        | Báo hiệu            | 1        |
| Nút nhấn MODE/SET/INC | Điều khiển          | 3        |

---

## 📌 **Sơ đồ chân**

```
ESP32
├── MAX7219: DIN=13, CLK=14, CS=12
├── LCD I2C: SDA=21, SCL=22
├── DHT11: DATA=27
├── Buttons: MODE=16, SET=17, INC=5
├── Buzzer: 32
└── LED: 33 → 220Ω → GND
```

---

## 💻 **Cài đặt**

### 1. Cài Arduino IDE hoặc PlatformIO

### 2. Thêm board ESP32

```
https://dl.espressif.com/dl/package_esp32_index.json
```

### 3. Cài thư viện

* LedControl
* RTC DS3231 (RTClib)
* DHT sensor library
* LiquidCrystal_I2C

### 4. Nạp chương trình

* Chọn board: **ESP32 Dev Module**
* Chọn COM
* Upload

---

## 📖 **Hướng dẫn sử dụng**

### Bảng điều khiển

| Nút  | Chức năng              |
| ---- | ---------------------- |
| MODE | Chuyển chế độ          |
| SET  | Vào chỉnh/Start-Stop   |
| INC  | Tăng giá trị/Lap/Reset |

### Tóm tắt 5 chế độ

* **Mode 1:** Hiển thị nhiệt độ & độ ẩm
* **Mode 2:** Hiển thị ngày giờ
* **Mode 3:** Đặt báo thức
* **Mode 4:** Bấm giờ, xem Lap
* **Mode 5:** Đếm ngược, reset nhanh

---

## 🏗️ **Kiến trúc hệ thống**

### ⏱️ Cooperative Scheduler

```
Timer 10ms → ISR → SCH_Update() → O(1)
                   ↓
           SCH_Dispatch_Tasks()
```

| Task          | Period | Mô tả             |
| ------------- | ------ | ----------------- |
| CheckButtons  | 50ms   | Quét nút          |
| UpdateDisplay | 100ms  | LED 7-seg         |
| UpdateLCD     | 100ms  | LCD               |
| LEDBlink      | 100ms  | Chớp LED          |
| CheckAlarm    | 1000ms | Kiểm tra báo thức |
| ReadSensors   | 2000ms | Đọc DHT11         |

---

## 📺 **Định dạng hiển thị**

### LED 7-Segment Format
```
Mode 1: [HH.MM] [DD.MM]     (Time & Date)
Mode 2: [TT.T°C] [HH.H%]    (Temp & Humidity)
Mode 3: [HH.MM] [HH.MM]     (Current Time | Alarm Time)
Mode 4: [HH.MM.SS.CC]       (Stopwatch)
Mode 5: [HH.MM.SS.CC]       (Countdown)
```

### LCD 16x2 Format
```
┌────────────────┐
│Temp: 25.5°C    │  Mode 1
│Humi: 67.2%     │
└────────────────┘

┌────────────────┐
│Time: 14:35:42  │  Mode 2
│Date: 25/12/2024│
└────────────────┘

┌────────────────┐
│Now: 14:35:42   │  Mode 3
│Alarm: 10:30    │
└────────────────┘

┌────────────────┐
│Stopwatch RUN   │  Mode 4
│01:23:45.67 L3  │
└────────────────┘

┌────────────────┐
│Countdown RUN   │  Mode 5
│00:03:00.00     │
└────────────────┘
```


---

## 📁 **Cấu trúc dự án**

```
smart-clock-esp32/
├── main.cpp
├── scheduler/
├── App_Tasks/
├── button_handler/
├── lcd_display/
├── led_7seg_display/
├── sensor_handler/
├── global_vars/
└── README.md
```

---

## 🐛 **Gỡ lỗi**

| Lỗi                | Nguyên nhân       | Khắc phục           |
| ------------------ | ----------------- | ------------------- |
| LCD không hiển thị | Sai địa chỉ I2C   | Thử 0x27 hoặc 0x3F  |
| DHT lỗi            | Nguồn/đấu dây kém | Kiểm tra lại wiring |
| Nút loạn           | Debounce thấp     | Tăng debounce       |

---

## 🚀 **Hướng phát triển**

* [ ] Đồng bộ thời gian NTP qua WiFi
* [ ] Giao tiếp MQTT
* [ ] Thêm nhiều báo thức
* [ ] Ghi log vào SD Card
* [ ] App điều khiển qua Bluetooth

---

## 👨‍💻 **Tác giả & License**

**Author:** *Phạm Công Võ*

* GitHub: **@vophamk23**
* Email: [congvolv1@gmail.com](mailto:your.email@example.com)

# 🕐 Smart Clock ESP32

### **Multi-functional Smart Clock System with ESP32**

<div align="center">

**A feature-rich embedded system with real-time scheduling, dual displays, and environmental monitoring**

<br>

![Smart Clock System](path/to/your/hero-image.jpg)

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
│                    ESP32 DevKit                      │
├─────────────────────────────────────────────────────┤
│                                                      │
│  MAX7219 LED 7-Segment:                             │
│    ├── DIN  → GPIO 13                               │
│    ├── CLK  → GPIO 14                               │
│    └── CS   → GPIO 12                               │
│                                                      │
│  LCD I2C 16x2:                                       │
│    ├── SDA  → GPIO 21                               │
│    └── SCL  → GPIO 22                               │
│                                                      │
│  DHT11 Sensor:                                       │
│    └── DATA → GPIO 27                               │
│                                                      │
│  Control Buttons:                                    │
│    ├── MODE → GPIO 16 (INPUT_PULLUP)                │
│    ├── SET  → GPIO 17 (INPUT_PULLUP)                │
│    └── INC  → GPIO 5  (INPUT_PULLUP)                │
│                                                      │
│  Audio/Visual Indicators:                            │
│    ├── Buzzer → GPIO 32                             │
│    └── LED    → GPIO 33 → 220Ω → GND                │
│                                                      │
│  Power:                                              │
│    ├── VIN  → 5V                                     │
│    ├── 3V3  → 3.3V output (for sensors)             │
│    └── GND  → Common ground                         │
│                                                      │
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
   └─────────┘          │  GPIO21 │←────────┼─── SDA  │
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

**Arduino IDE:** `Sketch → Include Library → Manage Libraries...`

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

### **3. Clone Repository**

```bash
git clone https://github.com/yourusername/smart-clock-esp32.git
cd smart-clock-esp32
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

### **5. Upload to ESP32**

**Arduino IDE:**
1. Connect ESP32 via USB
2. Select **Tools → Board → ESP32 Dev Module**
3. Select correct **Port**
4. Click **Upload**

**PlatformIO:**
```bash
pio run --target upload
```

### **6. Monitor Serial Output (Optional)**

```bash
# Arduino IDE: Tools → Serial Monitor (115200 baud)
# PlatformIO:
pio device monitor -b 115200
```

---

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

---

## 🏗️ **System Architecture**

### **⏱️ Cooperative Scheduler**

The system uses a **hardware timer-based cooperative scheduler** for efficient multitasking:

```
┌─────────────────────────────────────────────────────┐
│         ESP32 Hardware Timer (10ms tick)            │
└──────────────────────┬──────────────────────────────┘
                       │
                       ▼
            ┌──────────────────┐
            │  Timer ISR       │  ← O(1) execution
            │  SCH_Update()    │     (updates only first task)
            └────────┬─────────┘
                     │
                     ▼
            ┌──────────────────┐
            │ Main Loop        │
            │ SCH_Dispatch()   │  ← Executes ready tasks
            └────────┬─────────┘
                     │
         ┌───────────┴───────────┐
         ▼                       ▼
    ┌─────────┐            ┌─────────┐
    │ Task 1  │   ...      │ Task N  │
    └─────────┘            └─────────┘
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

### **Module Dependencies**

```
main.cpp
    ├── scheduler.cpp         (Core scheduling)
    ├── App_Tasks.cpp        (Task implementations)
    │       ├── button_handler.cpp
    │       ├── led_7seg_display.cpp
    │       ├── lcd_display.cpp
    │       └── sensor_handler.cpp
    ├── global_vars.cpp      (Shared state)
    └── config.h             (Pin & timing definitions)
```

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

### **Key Files Description**

| File | Purpose |
|------|---------|
| `main.cpp` | System initialization, scheduler setup, main loop |
| `scheduler.cpp` | Hardware timer ISR, O(1) task update, dispatcher |
| `App_Tasks.cpp` | All 6 task function implementations |
| `button_handler.cpp` | Button scanning, debouncing, event handling |
| `led_7seg_display.cpp` | MAX7219 control, digit formatting |
| `lcd_display.cpp` | LCD I2C control, text formatting, icons |
| `sensor_handler.cpp` | DHT11 & DS3231 initialization |
| `global_vars.cpp` | Shared state variables (mode, timers, laps, etc.) |
| `config.h` | Centralized pin mappings and timing constants |

---

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

### **Debugging Steps**

1. **Enable Serial Monitoring**
   ```cpp
   Serial.begin(115200);
   Serial.println("System started");
   ```

2. **Check I2C Devices**
   ```cpp
   // Add to setup()
   Wire.begin();
   byte error, address;
   for(address = 1; address < 127; address++) {
       Wire.beginTransmission(address);
       error = Wire.endTransmission();
       if (error == 0) {
           Serial.print("I2C device at 0x");
           Serial.println(address, HEX);
       }
   }
   ```

3. **Test Individual Components**
   - Upload example sketches from each library
   - Verify sensor readings in Serial Monitor
   - Test buttons with simple digitalWrite tests

4. **Monitor Task Execution**
   ```cpp
   // In App_Tasks.cpp
   void Task_CheckButtons() {
       Serial.println("Button task running");
       checkButtons(getLedControl());
   }
   ```

### **LED Error Codes**

If system fails to initialize, LED blinks error code:

| Blinks | Meaning |
|--------|---------|
| 1 | RTC not found |
| 2 | LCD not responding |
| 3 | DHT sensor error |
| 4 | Scheduler overflow |

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

### **Community Contributions Welcome!**

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

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
- 💼 LinkedIn: [Your Profile](https://linkedin.com/in/yourprofile)
- 🌐 Website: [yourwebsite.com](https://yourwebsite.com)

---

## 🙏 **Acknowledgments**

- **ESP32 Community** for excellent documentation
- **Adafruit** for RTClib and DHT libraries
- **Arduino** for the accessible development platform
- **Contributors** who helped improve this project

---

## ⭐ **Support**

If you find this project helpful, please consider:

- ⭐ **Starring** the repository
- 🐛 **Reporting** bugs and issues
- 💡 **Suggesting** new features
- 📢 **Sharing** with others

---

<div align="center">

**Made with ❤️ using ESP32**

![ESP32](https://img.shields.io/badge/ESP32-Powered-red?logo=espressif)
![Arduino](https://img.shields.io/badge/Arduino-Compatible-teal?logo=arduino)

[⬆ Back to Top](#-smart-clock-esp32)

</div>
