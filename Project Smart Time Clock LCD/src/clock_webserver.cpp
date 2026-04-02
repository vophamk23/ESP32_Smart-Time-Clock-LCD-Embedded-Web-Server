/*
 * clock_webserver.cpp
 * Tầng HTTP — chỉ xử lý request/response và serve file từ SPIFFS
 *
 * Phụ thuộc:
 *   - clock_data.h  : mọi logic dữ liệu
 *   - SPIFFS        : phục vụ index.html / style.css / app.js từ thư mục data/
 *
 * AP Mode  : kết nối WiFi "SmartClock" → http://192.168.4.1
 * STA Mode : đổi WEB_USE_AP = 0, giữ WiFi ON trong wifi_clock.cpp
 */

#include "clock_webserver.h"
#include "clock_data.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "config.h"

static WebServer server(WEB_PORT);

// ============================================================================
// HELPER: CORS + parse body
// ============================================================================
static void addCORS()
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// Lấy giá trị int của một key trong JSON body đơn giản
// VD: body = {"hour":7,"minute":30}  → getInt(body,"hour") = 7
static int getInt(const String &body, const char *key)
{
  String k = "\"";
  k += key;
  k += "\"";
  int idx = body.indexOf(k);
  if (idx < 0)
    return 0;
  int colon = body.indexOf(':', idx);
  if (colon < 0)
    return 0;
  return body.substring(colon + 1).toInt();
}

// ============================================================================
// SERVE STATIC FILES TỪ SPIFFS
// ============================================================================
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
// ROUTE HANDLERS — tất cả chỉ parse/validate rồi gọi clock_data
// ============================================================================

// GET / → index.html từ SPIFFS
void handleRoot()
{
  if (!serveFile("/index.html", "text/html"))
    server.send(404, "text/plain", "index.html not found. Upload SPIFFS data first.");
}

// GET /style.css
void handleCSS()
{
  if (!serveFile("/style.css", "text/css"))
    server.send(404, "text/plain", "style.css not found");
}

// GET /app.js
void handleJS()
{
  if (!serveFile("/app.js", "application/javascript"))
    server.send(404, "text/plain", "app.js not found");
}

// GET /api/status
void handleGetStatus()
{
  addCORS();
  server.send(200, "application/json", clockData_buildStatusJSON());
}

// POST /api/mode   {"mode":N}
void handleSetMode()
{
  addCORS();
  int m = getInt(server.arg("plain"), "mode");
  if (clockData_setMode(m))
    server.send(200, "application/json", "{\"ok\":true,\"mode\":" + String(m) + "}");
  else
    server.send(400, "application/json", "{\"error\":\"invalid mode\"}");
}

// POST /api/alarm  {"hour":H,"minute":M}
void handleSetAlarm()
{
  addCORS();
  String body = server.arg("plain");
  int h = getInt(body, "hour");
  int m = getInt(body, "minute");
  if (clockData_setAlarm(h, m))
    server.send(200, "application/json",
                "{\"ok\":true,\"hour\":" + String(h) + ",\"minute\":" + String(m) + "}");
  else
    server.send(400, "application/json", "{\"error\":\"out of range\"}");
}

// POST /api/alarm/stop
void handleStopAlarm()
{
  addCORS();
  clockData_stopAlarm();
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/stopwatch/start
void handleSWStart()
{
  addCORS();
  clockData_swStart();
  server.send(200, "application/json", "{\"ok\":true,\"running\":true}");
}

// POST /api/stopwatch/stop
void handleSWStop()
{
  addCORS();
  clockData_swStop();
  server.send(200, "application/json", "{\"ok\":true,\"running\":false}");
}

// POST /api/stopwatch/reset
void handleSWReset()
{
  addCORS();
  clockData_swReset();
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/countdown/set  {"hours":H,"minutes":M,"seconds":S}
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

// POST /api/countdown/start
void handleCDStart()
{
  addCORS();
  if (clockData_cdStart())
    server.send(200, "application/json", "{\"ok\":true}");
  else
    server.send(400, "application/json", "{\"error\":\"zero duration\"}");
}

// POST /api/countdown/stop
void handleCDStop()
{
  addCORS();
  clockData_cdStop();
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/countdown/reset
void handleCDReset()
{
  addCORS();
  clockData_cdReset();
  server.send(200, "application/json", "{\"ok\":true}");
}

// ============================================================================
// KHỞI TẠO
// ============================================================================
void initWebServer()
{
  // --- SPIFFS ---
  if (!SPIFFS.begin(true))
    Serial.println("[WEB] SPIFFS mount failed!");
  else
    Serial.println("[WEB] SPIFFS mounted OK");

  // --- WiFi ---
#if WEB_USE_AP
  WiFi.mode(WIFI_AP);
  (strlen(AP_PASS) >= 8) ? WiFi.softAP(AP_SSID, AP_PASS)
                         : WiFi.softAP(AP_SSID);
  Serial.printf("[WEB] AP SSID : %s\n", AP_SSID);
  Serial.printf("[WEB] AP IP   : %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("[WEB] URL     : http://%s\n", WiFi.softAPIP().toString().c_str());
#else
  Serial.printf("[WEB] STA IP  : %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("[WEB] URL     : http://%s\n", WiFi.localIP().toString().c_str());
#endif

  // --- Routes ---
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

  server.onNotFound([&]()
                    {
        if (server.method() == HTTP_OPTIONS) { addCORS(); server.send(204); }
        else server.send(404, "text/plain", "Not found"); });

  server.begin();
  Serial.printf("[WEB] HTTP server started port %d\n", WEB_PORT);
}

// ============================================================================
// SCHEDULER TASK
// ============================================================================
void Task_WebServer()
{
  server.handleClient();
}