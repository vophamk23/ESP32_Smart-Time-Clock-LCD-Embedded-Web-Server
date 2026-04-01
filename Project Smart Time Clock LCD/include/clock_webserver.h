#ifndef CLOCK_WEBSERVER_H
#define CLOCK_WEBSERVER_H

#include <Arduino.h>

// ============================================================================
// clock_webserver.h
// Chỉ lo HTTP: routes, parse request, gọi clock_data, trả JSON/file
// Không biết gì về RTC, cảm biến, global_vars
// ============================================================================

// Gọi trong setup() SAU initWiFiAndNTP()
void initWebServer();

// Task scheduler — SCH_Add_Task(Task_WebServer, 0, 5)
void Task_WebServer();

#endif // CLOCK_WEBSERVER_H