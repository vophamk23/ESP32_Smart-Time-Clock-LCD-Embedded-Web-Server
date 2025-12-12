#ifndef CONFIG_H
#define CONFIG_H

// ==================== ĐỊNH NGHĨA CHÂN KẾT NỐI ====================
#define DIN_PIN 13      // MAX7219 DIN
#define CLK_PIN 14      // MAX7219 CLK
#define CS_PIN 12       // MAX7219 CS
#define DHT_PIN 27      // DHT12 Data pin
#define BUZZER_PIN 32   // Chân kết nối buzzer
#define LED_PIN 33      // LED báo hiệu (GPIO 33)
#define BTN_MODE_PIN 16 // Nút nhấn chuyển mode
#define BTN_SET_PIN 17  // Nút cài đặt
#define BTN_INC_PIN 5   // Nút tăng giá trị

// ==================== LCD I2C PINS ====================
#define LCD_SDA_PIN 21   // I2C SDA cho LCD
#define LCD_SCL_PIN 22   // I2C SCL cho LCD
#define LCD_ADDRESS 0x27 // Địa chỉ I2C của LCD (có thể là 0x27 hoặc 0x3F)
#define LCD_COLS 16      // Số cột của LCD
#define LCD_ROWS 2       // Số hàng của LCD

// ==================== CÁC HẰNG SỐ ====================
#define BUTTON_DEBOUNCE 300 // Thời gian chống dội nút nhấn (ms)
#define DISPLAY_INTENSITY 8 // Độ sáng LED (0-15)
#define TOTAL_MODES 5       // Tổng số chế độ hiển thị

// ==================== ENUM CHẾ ĐỘ HIỂN THỊ ====================
enum DisplayMode
{
    MODE_TEMP_HUMIDITY = 0, // Nhiệt độ + Độ ẩm
    MODE_DATE_TIME = 1,     // Giờ + Ngày tháng
    MODE_ALARM = 2,         // Báo thức
    MODE_STOPWATCH = 3,     // Bấm giờ
    MODE_COUNTDOWN = 4      // Đếm ngược
};

#endif // CONFIG_H