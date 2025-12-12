#include "button_handler.h"
#include "config.h"
#include "global_vars.h"
#include "led_7seg_display.h"
#include "lcd_display.h"

// ============================================================================
// HÀM KHỞI TẠO CÁC NÚT BẤM VÀ BUZZER
// ============================================================================
// Chức năng: Cấu hình các chân nút bấm và buzzer
void initButtons()
{
    // Cấu hình các nút bấm ở chế độ INPUT_PULLUP
    // INPUT_PULLUP: Kích hoạt điện trở kéo lên nội bộ, nút bấm = LOW khi nhấn
    pinMode(BTN_MODE_PIN, INPUT_PULLUP); // Nút chuyển chế độ
    pinMode(BTN_SET_PIN, INPUT_PULLUP);  // Nút cài đặt/xác nhận
    pinMode(BTN_INC_PIN, INPUT_PULLUP);  // Nút tăng giá trị

    // Cấu hình chân buzzer ở chế độ OUTPUT
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Tắt buzzer khi khởi động

    // ========== LED ==========
    pinMode(LED_PIN, OUTPUT);   // Cấu hình chân LED là OUTPUT
    digitalWrite(LED_PIN, LOW); // Tắt LED khi khởi động
}

// ============================================================================
// XỬ LÝ NÚT MODE - CHUYỂN ĐỔI CHẾ ĐỘ HIỂN THỊ
// ============================================================================
// Chức năng: Chuyển đổi giữa các chế độ: Clock, Temp/Humidity, Alarm,
//            Stopwatch, Countdown
// Tham số: lc - con trỏ tới đối tượng LED 7-segment
void handleModeButton(LedControl *lc)
{
    // ========================================================================
    // CHỐNG DỘI NÚT (DEBOUNCE)
    // ========================================================================
    // Kiểm tra xem đã đủ thời gian giữa 2 lần nhấn chưa
    if (millis() - lastButtonPress < BUTTON_DEBOUNCE)
        return; // Nếu chưa đủ thời gian, bỏ qua lần nhấn này

    lastButtonPress = millis(); // Cập nhật thời điểm nhấn nút cuối cùng

    // ========================================================================
    // CHUYỂN CHẾ ĐỘ
    // ========================================================================
    // Tăng chế độ lên 1, quay vòng về 0 khi vượt quá TOTAL_MODES
    displayMode = (displayMode + 1) % TOTAL_MODES;
    // Ví dụ: TOTAL_MODES = 5 => 0→1→2→3→4→0→1...

    // In thông báo ra Serial Monitor để debug
    Serial.print("Mode changed -> ");
    Serial.println(displayMode);

    // ========================================================================
    // RESET CÁC TRẠNG THÁI KHI CHUYỂN CHẾ ĐỘ
    // ========================================================================

    // ------------------------------------------------------------------------
    // Nếu chuyển vào chế độ STOPWATCH
    // ------------------------------------------------------------------------
    if (displayMode == MODE_STOPWATCH)
    {
        timerCurrentTime = 0;   // Reset thời gian về 0
        isTimerRunning = false; // Dừng stopwatch
    }

    // ------------------------------------------------------------------------
    // Nếu chuyển vào chế độ COUNTDOWN
    // ------------------------------------------------------------------------
    else if (displayMode == MODE_COUNTDOWN)
    {
        countdownEditing = true;    // Vào chế độ chỉnh sửa
        isCountdownRunning = false; // Dừng countdown
        countdownTriggered = false; // Tắt cảnh báo hết giờ
        countdownEditField = 0;     // Chọn trường chỉnh sửa đầu tiên (giờ)

        // Khôi phục giá trị ban đầu
        c_hours = c_hours_initial;
        c_minutes = c_minutes_initial;
        c_seconds = c_seconds_initial;
    }

    // ========================================================================
    // XÓA MÀN HÌNH LED VÀ LCD
    // ========================================================================
    lc->clearDisplay(0); // Xóa LED 7-segment
    getLCD()->clear();   // Xóa LCD 16x2
}

// ============================================================================
// XỬ LÝ NÚT SET - CÀI ĐẶT/XÁC NHẬN
// ============================================================================
// Chức năng: Xử lý các thao tác cài đặt tùy theo chế độ hiện tại
void handleSetButton()
{
    // ========================================================================
    // CHỐNG DỘI NÚT
    // ========================================================================
    if (millis() - lastButtonPress < BUTTON_DEBOUNCE)
        return;
    lastButtonPress = millis();

    // ========================================================================
    // CHẾ ĐỘ ALARM: CHUYỂN ĐỔI GIỮA CHỈNH GIỜ VÀ CHỈNH PHÚT
    // ========================================================================
    if (displayMode == MODE_ALARM)
    {
        // Đổi trạng thái giữa chỉnh giờ và chỉnh phút
        alarmEditHour = !alarmEditHour;

        Serial.print("Alarm edit -> ");
        Serial.println(alarmEditHour ? "HOUR" : "MINUTE");

        // Nếu báo thức đang kêu, tắt nó đi
        if (alarmTriggered)
        {
            digitalWrite(BUZZER_PIN, LOW);  // Tắt buzzer
            digitalWrite(LED_PIN, LOW);     // Tắt LED báo hiệu
            alarmTriggered = false;         // Tắt cờ báo thức
            alarmEditHour = !alarmEditHour; // Đổi lại trạng thái chỉnh sửa
            Serial.println("Alarm stopped");
        }
    }

    // ========================================================================
    // CHẾ ĐỘ STOPWATCH: START/STOP VÀ LƯU LAP
    // ========================================================================
    else if (displayMode == MODE_STOPWATCH)
    {
        // --------------------------------------------------------------------
        // NẾU STOPWATCH ĐANG DỪNG → BẮT ĐẦU CHẠY
        // --------------------------------------------------------------------
        if (!isTimerRunning)
        {
            // Tính lại thời điểm bắt đầu dựa trên thời gian đã trôi qua
            // Điều này cho phép tiếp tục từ thời điểm dừng (pause/resume)
            timerStartTime = millis() - timerCurrentTime;
            isTimerRunning = true;
            Serial.println("Stopwatch START");
        }

        // --------------------------------------------------------------------
        // NẾU STOPWATCH ĐANG CHẠY → DỪNG LẠI VÀ LƯU LAP
        // --------------------------------------------------------------------
        else
        {
            isTimerRunning = false;
            Serial.println("Stopwatch STOP -> save lap");

            // Kiểm tra xem còn chỗ lưu lap không (tối đa 5 laps)
            if (lapCount < 5)
            {
                laps[lapCount++] = timerCurrentTime; // Lưu thời gian lap hiện tại

                Serial.print("Saved lap ");
                Serial.print(lapCount);
                Serial.print(": ");
                Serial.println(timerCurrentTime);
            }
            else
            {
                Serial.println("Lap storage full"); // Đã đầy, không lưu được
            }
        }
    }

    // ========================================================================
    // CHẾ ĐỘ COUNTDOWN: XÁC NHẬN TRƯỜNG CHỈNH SỬA HOẶC BẮT ĐẦU
    // ========================================================================
    else if (displayMode == MODE_COUNTDOWN)
    {
        // --------------------------------------------------------------------
        // NẾU COUNTDOWN ĐÃ HẾT GIỜ (ĐANG KÊU) → TẮT VÀ RESET
        // --------------------------------------------------------------------
        if (countdownTriggered)
        {
            digitalWrite(BUZZER_PIN, LOW); // Tắt buzzer
            digitalWrite(LED_PIN, LOW);
            countdownTriggered = false; // Tắt cờ hết giờ
            countdownEditing = true;    // Quay lại chế độ chỉnh sửa
            isCountdownRunning = false; // Dừng countdown

            // Khôi phục giá trị ban đầu
            c_hours = c_hours_initial;
            c_minutes = c_minutes_initial;
            c_seconds = c_seconds_initial;

            Serial.println("Countdown stopped by SET");
            return; // Thoát khỏi hàm
        }

        // --------------------------------------------------------------------
        // NẾU ĐANG CHỈNH SỬA
        // --------------------------------------------------------------------
        if (countdownEditing)
        {
            // Nếu chưa chỉnh hết 3 trường (giờ, phút, giây)
            if (countdownEditField < 2)
            {
                countdownEditField++; // Chuyển sang trường tiếp theo

                Serial.print("Countdown edit field -> ");
                Serial.println(countdownEditField);
                // 0 = giờ, 1 = phút, 2 = giây
            }

            // --------------------------------------------------------------------
            // ĐÃ CHỈNH HẾT 3 TRƯỜNG → BẮT ĐẦU COUNTDOWN
            // --------------------------------------------------------------------
            else
            {
                // Tính tổng thời gian countdown theo giây
                unsigned long totalSeconds = (unsigned long)c_hours * 3600UL +
                                             (unsigned long)c_minutes * 60UL +
                                             (unsigned long)c_seconds;

                // Kiểm tra nếu thời gian = 0, không khởi động
                if (totalSeconds == 0)
                {
                    Serial.println("Countdown duration is zero, not started");
                }
                else
                {
                    // Chuyển đổi sang milliseconds
                    countdownDuration = totalSeconds * 1000UL;
                    countdownStartTime = millis(); // Lưu thời điểm bắt đầu
                    isCountdownRunning = true;     // Bắt đầu chạy
                    countdownEditing = false;      // Thoát chế độ chỉnh sửa

                    // Lưu giá trị ban đầu để có thể reset
                    c_hours_initial = c_hours;
                    c_minutes_initial = c_minutes;
                    c_seconds_initial = c_seconds;

                    Serial.print("Countdown started: ");
                    Serial.println(countdownDuration);
                }
            }
        }
    }
}

// ============================================================================
// XỬ LÝ NÚT INC - TĂNG GIÁ TRỊ/RESET/XEM LAPS
// ============================================================================
// Chức năng: Tăng giá trị cài đặt, reset countdown, hoặc xem laps
void handleIncButton()
{
    // ========================================================================
    // CHỐNG DỘI NÚT
    // ========================================================================
    if (millis() - lastButtonPress < BUTTON_DEBOUNCE)
        return;
    lastButtonPress = millis();

    // ========================================================================
    // CHẾ ĐỘ ALARM: TĂNG GIỜ HOẶC PHÚT
    // ========================================================================
    if (displayMode == MODE_ALARM)
    {
        // Nếu đang chỉnh giờ
        if (alarmEditHour)
        {
            alarmHour = (alarmHour + 1) % 24; // Tăng giờ (0-23, quay vòng)
        }
        // Nếu đang chỉnh phút
        else
        {
            alarmMinute = (alarmMinute + 1) % 60; // Tăng phút (0-59, quay vòng)
        }
    }

    // ========================================================================
    // CHẾ ĐỘ STOPWATCH: XEM CÁC LAP ĐÃ LƯU
    // ========================================================================
    else if (displayMode == MODE_STOPWATCH)
    {
        // Hiển thị danh sách các lap đã lưu trên LED 7-segment và LCD
        showSavedLaps(getLedControl());
    }

    // ========================================================================
    // CHẾ ĐỘ COUNTDOWN: TĂNG GIÁ TRỊ HOẶC RESET
    // ========================================================================
    else if (displayMode == MODE_COUNTDOWN)
    {
        // --------------------------------------------------------------------
        // NẾU ĐANG CHẠY HOẶC ĐÃ HẾT GIỜ → RESET HOÀN TOÀN
        // --------------------------------------------------------------------
        if (isCountdownRunning || countdownTriggered)
        {
            isCountdownRunning = false;    // Dừng countdown
            countdownTriggered = false;    // Tắt cảnh báo
            digitalWrite(BUZZER_PIN, LOW); // Tắt buzzer
            digitalWrite(LED_PIN, LOW);    // Tắt LED báo hiệu

            // Reset về chế độ chỉnh sửa và xóa hết giá trị
            countdownEditing = true;
            countdownEditField = 0;

            // Đặt tất cả giá trị về 0
            c_hours = 0;
            c_minutes = 0;
            c_seconds = 0;
            c_hours_initial = 0;
            c_minutes_initial = 0;
            c_seconds_initial = 0;
            countdownDuration = 0;
            countdownRemaining = 0;

            Serial.println("Countdown reset to 00:00:00 and edit mode");
            return; // Thoát khỏi hàm
        }

        // --------------------------------------------------------------------
        // NẾU ĐANG CHỈNH SỬA → TĂNG GIÁ TRỊ TRƯỜNG ĐANG CHỌN
        // --------------------------------------------------------------------
        if (countdownEditing)
        {
            // Tăng giờ (0-99)
            if (countdownEditField == 0)
            {
                c_hours = (c_hours + 1) % 100;
            }
            // Tăng phút (0-59)
            else if (countdownEditField == 1)
            {
                c_minutes = (c_minutes + 1) % 60;
            }
            // Tăng giây (bước nhảy 10 giây: 0, 10, 20, 30, 40, 50, 0...)
            else if (countdownEditField == 2)
            {
                c_seconds = (c_seconds + 10) % 60;
            }

            // Cập nhật giá trị ban đầu để có thể reset về đây
            c_hours_initial = c_hours;
            c_minutes_initial = c_minutes;
            c_seconds_initial = c_seconds;

            // In giá trị đã chỉnh sửa ra Serial Monitor
            Serial.print("Countdown edited -> ");
            Serial.print(c_hours);
            Serial.print(":");
            Serial.print(c_minutes);
            Serial.print(":");
            Serial.println(c_seconds);
        }
    }
}

// ============================================================================
// HÀM KIỂM TRA VÀ XỬ LÝ TẤT CẢ CÁC NÚT
// ============================================================================
// Chức năng: Được gọi liên tục trong loop() để kiểm tra trạng thái các nút
// Tham số: lc - con trỏ tới đối tượng LED 7-segment
void checkButtons(LedControl *lc)
{
    // Kiểm tra nút MODE (chân = LOW khi nhấn do dùng INPUT_PULLUP)
    if (digitalRead(BTN_MODE_PIN) == LOW)
    {
        handleModeButton(lc);
    }

    // Kiểm tra nút SET
    if (digitalRead(BTN_SET_PIN) == LOW)
    {
        handleSetButton();
    }

    // Kiểm tra nút INC
    if (digitalRead(BTN_INC_PIN) == LOW)
    {
        handleIncButton();
    }
}