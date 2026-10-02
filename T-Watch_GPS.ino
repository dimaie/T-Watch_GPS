/**
 * @file      T-Watch_GPS.ino
 * @brief     Green-on-Black Tactical Watch with Clock and GPS modes for LilyGo T-Watch Ultra
 * @license   MIT
 */
#include "AnalogClock.h"
#include "GpsView.h"
#include "WatchModeManager.h"
#include "HttpServerHandler.h"
#include "UartCommandHandler.h"
#include "WatchCommandController.h"
#include "WatchGlobals.h"
#include <LV_Helper.h>
#include <LilyGoLib.h>

// Uncomment to enable startup boot debug messages on serial output
// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINTF(...)
#endif

// Define global singletons declared in WatchGlobals.h
AnalogClock analogClock;
GpsView gpsView;
WatchModeManager modeManager;
WatchCommandController commandController;
HttpServerHandler httpHandler;
static UartCommandHandler uartHandler;

void setup() {
  Serial.begin(115200);

#ifdef DEBUG
  uint32_t waitSerialMs = millis();
  while (!Serial && (millis() - waitSerialMs < 3000)) {
    delay(10);
  }
  delay(500);

  DEBUG_PRINTLN(F("\r\n[BOOT] =============================================="));
  DEBUG_PRINTLN(F("[BOOT] T-Watch Ultra Booting Up..."));
  DEBUG_PRINTLN(F("[BOOT] =============================================="));
#endif

  DEBUG_PRINTLN(F("[BOOT] [Step 1/7] Initializing hardware instance (instance.begin())..."));
  instance.begin();
  DEBUG_PRINTLN(F("[BOOT] [Step 1/7] OK: Hardware instance initialized successfully."));

  DEBUG_PRINTLN(F("[BOOT] [Step 2/7] Initializing LVGL display helper (beginLvglHelper)..."));
  beginLvglHelper(instance);
  DEBUG_PRINTLN(F("[BOOT] [Step 2/7] OK: LVGL display helper initialized."));

  DEBUG_PRINTLN(F("[BOOT] [Step 3/7] Setting display brightness to MAX..."));
  instance.setBrightness(DEVICE_MAX_BRIGHTNESS_LEVEL);
  DEBUG_PRINTLN(F("[BOOT] [Step 3/7] OK: Brightness configured."));

  DEBUG_PRINTLN(F("[BOOT] [Step 4/7] Creating Mode Views & Watch Mode Manager..."));
  analogClock.create();
  gpsView.create();

  IWatchView *views[] = { &analogClock, &gpsView };
  modeManager.create(views, sizeof(views) / sizeof(views[0]));
  DEBUG_PRINTLN(F("[BOOT] [Step 4/7] OK: Mode Views & Watch Mode Manager created."));

  DEBUG_PRINTLN(F("[BOOT] [Step 5/7] Initializing Core Command Controller..."));
  commandController.begin();
  DEBUG_PRINTLN(F("[BOOT] [Step 5/7] OK: Core Command Controller initialized."));

  DEBUG_PRINTLN(F("[BOOT] [Step 6/7] Initializing UART Command Handler..."));
  uartHandler.begin(&commandController, Serial);
  DEBUG_PRINTLN(F("[BOOT] [Step 6/7] OK: UART Command Handler initialized."));

  DEBUG_PRINTLN(F("[BOOT] [Step 7/7] Initializing HTTP Server Handler..."));
  httpHandler.begin(&commandController);
  DEBUG_PRINTLN(F("[BOOT] [Step 7/7] OK: HTTP Server Handler initialized (disabled by default)."));

  DEBUG_PRINTLN(F("[BOOT] =============================================="));
  DEBUG_PRINTLN(F("[BOOT] Boot Sequence Complete! Main loop running."));
  DEBUG_PRINTLN(F("[BOOT] ==============================================\r\n"));
}

void loop() {
  // Process UART input and HTTP server events
  uartHandler.process();
  httpHandler.process();

  // Process LVGL GUI tasks
  lv_timer_handler();
  delay(2);
}
