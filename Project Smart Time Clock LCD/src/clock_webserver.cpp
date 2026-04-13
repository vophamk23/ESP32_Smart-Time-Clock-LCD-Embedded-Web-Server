/*
 * clock_webserver.cpp
 * Tầng HTTP — chỉ xử lý request/response và serve file từ SPIFFS
 *
 * Phụ thuộc:
 *   - clock_data.h  : mọi logic dữ liệu (không có HTTP ở đây)
 *   - SPIFFS        : phục vụ index.html / style.css / app.js từ thư mục data/
 *
 * AP Mode  : kết nối WiFi "SmartClock" → http://192.168.4.1
 * STA Mode : đổi WEB_USE_AP = 0, giữ WiFi ON trong wifi_clock.cpp
 *
 */

#include "clock_webserver.h"
#include "clock_data.h"
#include "global_vars.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "config.h"

static WebServer server(WEB_PORT);

// ============================================================================
// HELPER: Thêm CORS header vào response
// ============================================================================
// Cho phép browser từ bất kỳ origin nào gọi API này (cần thiết khi
// frontend chạy trên domain/port khác với ESP32)
static void addCORS()
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ============================================================================
// HELPER: Parse giá trị int từ JSON body đơn giản
// ============================================================================
// Tìm key trong chuỗi JSON và trả về giá trị int sau dấu ':'
// VD: body = {"hour":7,"minute":30} → getInt(body, "hour") = 7
static int getInt(const String &body, const char *key)
{
  // Tạo chuỗi tìm kiếm có dấu ngoặc kép: "hour"
  String k = "\"";
  k += key;
  k += "\"";

  int idx = body.indexOf(k);
  if (idx < 0)
    return 0; // Không tìm thấy key

  // Vị trí ngay sau key (ký tự '"' đóng)
  int afterKey = idx + k.length();

  // Bỏ qua khoảng trắng giữa key và dấu ':'
  while (afterKey < (int)body.length() && body[afterKey] == ' ')
    afterKey++;

  // Ký tự tiếp theo PHẢI là ':' — nếu không phải thì không phải key này
  // VD: "hours" sẽ có ký tự 's' ở đây, không phải ':' → bỏ qua đúng
  if (afterKey >= (int)body.length() || body[afterKey] != ':')
    return 0;

  // Lấy giá trị số sau dấu ':'
  return body.substring(afterKey + 1).toInt();
}

// ============================================================================
// HELPER: Serve file tĩnh từ SPIFFS
// ============================================================================
// Mở file từ SPIFFS và stream về browser với Content-Type phù hợp
// Trả về false nếu file không tồn tại (để caller gửi 404)
static bool serveFile(const String &path, const char *contentType)
{
  if (!SPIFFS.exists(path))
    return false;

  File f = SPIFFS.open(path, "r");
  server.streamFile(f, contentType);
  f.close();
  return true;
}

// ============================================================================
// ROUTE: GET / → index.html
// ============================================================================
// Trả về trang web chính từ SPIFFS
// Nếu chưa upload data/ bằng PlatformIO → hiển thị hướng dẫn
void handleRoot()
{
  if (!serveFile("/index.html", "text/html"))
    server.send(404, "text/plain",
                "index.html not found. Upload SPIFFS data first.");
}

// ============================================================================
// ROUTE: GET /style.css
// ============================================================================
void handleCSS()
{
  if (!serveFile("/style.css", "text/css"))
    server.send(404, "text/plain", "style.css not found");
}

// ============================================================================
// ROUTE: GET /app.js
// ============================================================================
void handleJS()
{
  if (!serveFile("/app.js", "application/javascript"))
    server.send(404, "text/plain", "app.js not found");
}

// ============================================================================
// ROUTE: GET /api/status → trả về JSON trạng thái toàn bộ đồng hồ
// ============================================================================
// Trả về JSON gồm: time, sensor, mode, alarm, stopwatch, countdown, system
void handleGetStatus()
{
  addCORS();

  static String s_cache;           // Bộ nhớ đệm JSON
  static uint32_t s_cacheTime = 0; // Thời điểm cache lần cuối

  // Chỉ build lại JSON khi cache rỗng hoặc đã quá 500ms
  if (s_cache.isEmpty() || millis() - s_cacheTime >= 500)
  {
    s_cache = clockData_buildStatusJSON();
    s_cacheTime = millis();
  }

  server.send(200, "application/json", s_cache);
}

// ============================================================================
// ROUTE: POST /api/mode  body: {"mode": N}
// ============================================================================
// Chuyển đổi chế độ hiển thị (0=TEMP, 1=DATETIME, 2=ALARM, 3=SW, 4=CD)
// clockData_setMode() validate range và reset state phù hợp
void handleSetMode()
{
  addCORS();
  int m = getInt(server.arg("plain"), "mode");

  if (clockData_setMode(m))
    server.send(200, "application/json",
                "{\"ok\":true,\"mode\":" + String(m) + "}");
  else
    server.send(400, "application/json",
                "{\"error\":\"invalid mode\"}");
}

// ============================================================================
// ROUTE: POST /api/alarm  body: {"hour": H, "minute": M}
// ============================================================================
// Cài đặt giờ báo thức và lưu vào Preferences (flash)
// clockData_setAlarm() validate range 0-23 / 0-59
void handleSetAlarm()
{
  addCORS();
  String body = server.arg("plain");
  int h = getInt(body, "hour");
  int m = getInt(body, "minute");

  if (clockData_setAlarm(h, m))
    server.send(200, "application/json",
                "{\"ok\":true,\"hour\":" + String(h) +
                    ",\"minute\":" + String(m) + "}");
  else
    server.send(400, "application/json",
                "{\"error\":\"out of range\"}");
}

// ============================================================================
// ROUTE: POST /api/alarm/stop
// ============================================================================
// Tắt báo thức đang kêu: tắt buzzer, tắt LED, reset cờ alarmTriggered
void handleStopAlarm()
{
  addCORS();
  clockData_stopAlarm();
  server.send(200, "application/json", "{\"ok\":true}");
}

// ============================================================================
// ROUTE: POST /api/stopwatch/start
// ============================================================================
// Bắt đầu hoặc tiếp tục bấm giờ từ thời điểm dừng trước đó (pause/resume)
void handleSWStart()
{
  addCORS();
  clockData_swStart();
  server.send(200, "application/json", "{\"ok\":true,\"running\":true}");
}

// ============================================================================
// ROUTE: POST /api/stopwatch/stop
// ============================================================================
// Dừng bấm giờ và lưu lap hiện tại (tối đa 5 laps)
void handleSWStop()
{
  addCORS();
  clockData_swStop();
  server.send(200, "application/json", "{\"ok\":true,\"running\":false}");
}

// ============================================================================
// ROUTE: POST /api/stopwatch/reset
// ============================================================================
// Reset toàn bộ stopwatch: xóa thời gian, xóa tất cả laps
void handleSWReset()
{
  addCORS();
  clockData_swReset();
  server.send(200, "application/json", "{\"ok\":true}");
}

// ============================================================================
// ROUTE: POST /api/countdown/set  body: {"hours": H, "minutes": M, "seconds": S}
// ============================================================================
// Cài đặt thời gian đếm ngược, chuyển sang chế độ editing
// Giá trị được constrain: hours 0-99, minutes/seconds 0-59
void handleCDSet()
{
  addCORS();
  String body = server.arg("plain");

  clockData_cdSet(
      getInt(body, "hours"),
      getInt(body, "minutes"),
      getInt(body, "seconds"));

  server.send(200, "application/json", "{\"ok\":true}");
}

// ============================================================================
// ROUTE: POST /api/countdown/start
// ============================================================================
// Bắt đầu đếm ngược với thời gian đã cài đặt
// Trả về 400 nếu thời gian = 0 (chưa cài hoặc cài sai)
void handleCDStart()
{
  addCORS();

  if (clockData_cdStart())
    server.send(200, "application/json", "{\"ok\":true}");
  else
    server.send(400, "application/json",
                "{\"error\":\"zero duration\"}");
}

// ============================================================================
// ROUTE: POST /api/countdown/stop
// ============================================================================
// Dừng countdown đang chạy
void handleCDStop()
{
  addCORS();
  clockData_cdStop();

  // Tắt buzzer và LED nếu countdown đang trong trạng thái kêu
  if (countdownTriggered)
  {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    countdownTriggered = false; // Reset cờ
    countdownEditing = true;    // Quay lại chế độ chỉnh sửa
    Serial.println("[WEB] Countdown triggered → stopped via web");
  }

  server.send(200, "application/json", "{\"ok\":true}");
}

// ============================================================================
// ROUTE: POST /api/countdown/reset
// ============================================================================
// Reset toàn bộ countdown: tắt buzzer/LED, xóa thời gian, về chế độ editing
void handleCDReset()
{
  addCORS();
  clockData_cdReset();
  server.send(200, "application/json", "{\"ok\":true}");
}

// ============================================================================
// KHỞI TẠO WEBSERVER
// ============================================================================
// Gọi 1 lần trong setup() — mount SPIFFS, cấu hình WiFi, đăng ký routes
void initWebServer()
{
  // --- Mount SPIFFS để serve file tĩnh ---
  if (!SPIFFS.begin(true))
    Serial.println("[WEB] SPIFFS mount failed!");
  else
    Serial.println("[WEB] SPIFFS mounted OK");

  // --- Cấu hình WiFi theo mode ---
#if WEB_USE_AP
  // AP mode: ESP32 tự tạo WiFi riêng, không cần router
  WiFi.mode(WIFI_AP);
  (strlen(AP_PASS) >= 8) ? WiFi.softAP(AP_SSID, AP_PASS)
                         : WiFi.softAP(AP_SSID);
  Serial.printf("[WEB] AP SSID : %s\n", AP_SSID);
  Serial.printf("[WEB] AP IP   : %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("[WEB] URL     : http://%s\n", WiFi.softAPIP().toString().c_str());
#else
  // STA mode: ESP32 kết nối vào WiFi nhà, lấy IP từ router
  Serial.printf("[WEB] STA IP  : %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("[WEB] URL     : http://%s\n", WiFi.localIP().toString().c_str());
#endif

  // --- Đăng ký tất cả routes ---
  server.on("/", HTTP_GET, handleRoot);
  server.on("/style.css", HTTP_GET, handleCSS);
  server.on("/app.js", HTTP_GET, handleJS);
  server.on("/api/status", HTTP_GET, handleGetStatus);
  server.on("/api/mode", HTTP_POST, handleSetMode);
  server.on("/api/alarm", HTTP_POST, handleSetAlarm);
  server.on("/api/alarm/stop", HTTP_POST, handleStopAlarm);
  server.on("/api/stopwatch/start", HTTP_POST, handleSWStart);
  server.on("/api/stopwatch/stop", HTTP_POST, handleSWStop);
  server.on("/api/stopwatch/reset", HTTP_POST, handleSWReset);
  server.on("/api/countdown/set", HTTP_POST, handleCDSet);
  server.on("/api/countdown/start", HTTP_POST, handleCDStart);
  server.on("/api/countdown/stop", HTTP_POST, handleCDStop);
  server.on("/api/countdown/reset", HTTP_POST, handleCDReset);

  // --- Xử lý route không tồn tại ---
  // OPTIONS request: browser gửi trước POST để check CORS → trả 204
  // Các request khác không tìm thấy → trả 404
  server.onNotFound([&]()
                    {
        if (server.method() == HTTP_OPTIONS)
        {
            addCORS();
            server.send(204);
        }
        else
        {
            server.send(404, "text/plain", "Not found");
        } });

  server.begin();
  Serial.printf("[WEB] HTTP server started port %d\n", WEB_PORT);
}

// ============================================================================
// SCHEDULER TASK: Xử lý HTTP request
// ============================================================================
// Được gọi mỗi 50ms từ cooperative scheduler (Task_WebServer_Handler)
// handleClient() kiểm tra có request đến không và xử lý ngay trong lần gọi đó
// Không block — nếu không có request thì trả về ngay lập tức
void Task_WebServer()
{
  server.handleClient();
}