# T-Watch_GPS Project Summary

This document provides a comprehensive summary of the **T-Watch_GPS** firmware codebase for the LilyGo T-Watch Ultra. It is designed as a quick-reference guide for future AI assistant sessions.

---

## 1. Project Overview & Hardware Target

- **Device**: LilyGo T-Watch Ultra (ESP32-S3 microcontroller, ST7789 display, PCF8563 RTC, U-Blox GPS module via `Serial1`, 802.11 b/g/n Wi-Fi radio).
- **GUI Framework**: LVGL v9 via `LV_Helper.h` and `LilyGoLib.h`.
- **Design Theme**: Tactical green-on-black aesthetic (`0x00FF66` accents, `0x000000` dark background, rounded borders).

---

## 2. Core Architecture & Design Principles

### A. Abstract View Interface (`IWatchView`)
- **[IWatchView.h](file:///c:/Work/T-Watch_GPS/IWatchView.h)** defines a common interface for all watch UI mode views:
  - `virtual void create(lv_obj_t *parent = nullptr) = 0`: Instantiates LVGL widgets inside the view's root container.
  - `virtual void update() = 0`: Periodic update callback (hand movement, satellite polling, coordinates refresh).
  - `virtual lv_obj_t *getContainer() const = 0`: Returns root LVGL container.
  - `virtual const char *getModeName() const = 0`: Returns human-readable mode name (`"CLOCK"`, `"GPS"`).
  - `virtual void setEnabled(bool enabled)` / `virtual bool isEnabled() const`: Polymorphic enablement control.

### B. Concrete View Implementations
1. **[`AnalogClock`](file:///c:/Work/T-Watch_GPS/AnalogClock.h)** ([`AnalogClock.cpp`](file:///c:/Work/T-Watch_GPS/AnalogClock.cpp)):
   - Green-on-black analog watch face with RTC time updates every 500 ms.
   - Outer dial ring, 60 tick marks (hour/minute), hour numerals (12, 1, 2... 11), hour/minute/second hands, date readout label, center cap, and upper-left status line showing service status letters ('W' for Wi-Fi, 'G' for GPS, 'H' for WebServer in fixed slots) and battery charge percentage.
2. **[`GpsView`](file:///c:/Work/T-Watch_GPS/GpsView.h)** ([`GpsView.cpp`](file:///c:/Work/T-Watch_GPS/GpsView.cpp)):
   - Live GPS mode displaying status, tracked satellite count (`satellites.value()`), fix satellites state (`3D LOCK` / `SEARCHING...`), and current location coordinates (`Latitude`, `Longitude`, `Altitude`).
   - Displays a clean status message if GPS power is OFF.

### C. Operational Mode Manager (`WatchModeManager`)
- **[WatchModeManager.h](file:///c:/Work/T-Watch_GPS/WatchModeManager.h)** & **[WatchModeManager.cpp](file:///c:/Work/T-Watch_GPS/WatchModeManager.cpp)**:
  - Polymorphically manages an array of registered `IWatchView*` instances.
  - **Swipe UP Gesture**: Listens for `LV_EVENT_GESTURE` (`LV_DIR_TOP`) on any view or screen, displaying the **SELECT MODE** popup modal dialog.
  - **Global Sleep/Wake Power Policy**:
    - Centralized display timeout tracking (`m_timeoutMs`, `m_lastActivityMs`, `m_isScreenOn`).
    - Listens for `LV_EVENT_PRESSED` across all mode containers to reset the inactivity timer and wake display backlight if sleeping.
    - Automatically sleeps display after inactivity timeout across all modes.

### D. Centralized Command Controller (`WatchCommandController`)
- **[WatchCommandController.h](file:///c:/Work/T-Watch_GPS/WatchCommandController.h)** & **[WatchCommandController.cpp](file:///c:/Work/T-Watch_GPS/WatchCommandController.cpp)**:
  - Uses an array-based lookup table (`COMMAND_TABLE`) mapping command strings to handler member functions.
  - Shared engine for both UART Serial input and HTTP Web Server commands.
  - Handles Wi-Fi connection with NVS profile persistence (`Preferences` under `"wificfg"` namespace), device configuration persistence (`Preferences` under `"watchcfg"` namespace for display brightness), NTP time sync, GPS module power toggle, GPS time sync, timezone offsets, web server control, display timeout, display brightness, and mode switching.

### E. Input & Transport Handlers
- **[`UartCommandHandler`](file:///c:/Work/T-Watch_GPS/UartCommandHandler.h)** ([`UartCommandHandler.cpp`](file:///c:/Work/T-Watch_GPS/UartCommandHandler.cpp)): Non-blocking line parser for Serial input at 115200 baud.
- **[`HttpServerHandler`](file:///c:/Work/T-Watch_GPS/HttpServerHandler.h)** ([`HttpServerHandler.cpp`](file:///c:/Work/T-Watch_GPS/HttpServerHandler.cpp)): Embedded web server (disabled by default) listening on port 80. Accepts GET/POST requests at `/cmd?line=...`.

### F. Global Firmware Singletons (`WatchGlobals`)
- **[WatchGlobals.h](file:///c:/Work/T-Watch_GPS/WatchGlobals.h)** declares `extern` singletons (`analogClock`, `gpsView`, `modeManager`, `commandController`, `httpHandler`) instantiated at top level in **[T-Watch_GPS.ino](file:///c:/Work/T-Watch_GPS/T-Watch_GPS.ino)** to prevent constructor parameter bloat.

---

## 3. Available Commands Reference

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `help` | None | Displays list of available commands. |
| `set time` | `YYYY-MM-DD HH:MM:SS` | Manually updates RTC date and time. |
| `connect wifi` | `[SSID/Profile] [Pass]` | Connects to Wi-Fi network and saves profile to NVS. |
| `scan wifi` | None | Scans and lists nearby Wi-Fi SSIDs and RSSI levels. |
| `list wifi` | None | Displays saved Wi-Fi profiles stored in NVS. |
| `disconnect wifi` | None | Disconnects Wi-Fi and powers off radio. |
| `set timeout` | `<seconds>` | Sets global display auto-sleep timeout across all modes. |
| `set brightness` / `brightness` | `<0-255\|0-100%>` | Sets display backlight brightness level and persists to NVS. |
| `sync ntp` | `[server]` | Syncs RTC time via NTP (requires Wi-Fi). |
| `enable gps` | None | Powers on GPS hardware module and initializes serial parsing. |
| `disable gps` | None | Powers off GPS hardware module. |
| `set timezone` | `<offset_sec_or_hours>` | Sets local timezone offset for GPS/NTP time sync. |
| `sync gps` | None | Syncs RTC time from GPS satellite lock. |
| `enable webserver` | None | Enables HTTP web server on port 80. |
| `disable webserver` | None | Stops HTTP web server. |
| `set mode` / `mode` | `<clock\|gps>` | Switches active watch mode. |

---

## 4. Key Codebase Files Sitemap

- **[`T-Watch_GPS.ino`](file:///c:/Work/T-Watch_GPS/T-Watch_GPS.ino)**: Main sketch entry point, initializes hardware, instantiates global singletons, builds UI, and runs main loop.
- **[`WatchGlobals.h`](file:///c:/Work/T-Watch_GPS/WatchGlobals.h)**: Global singleton header declarations.
- **[`IWatchView.h`](file:///c:/Work/T-Watch_GPS/IWatchView.h)**: Abstract view interface.
- **[`AnalogClock.h`](file:///c:/Work/T-Watch_GPS/AnalogClock.h)** / **[`AnalogClock.cpp`](file:///c:/Work/T-Watch_GPS/AnalogClock.cpp)**: Clock UI implementation.
- **[`GpsView.h`](file:///c:/Work/T-Watch_GPS/GpsView.h)** / **[`GpsView.cpp`](file:///c:/Work/T-Watch_GPS/GpsView.cpp)**: GPS view implementation.
- **[`WatchModeManager.h`](file:///c:/Work/T-Watch_GPS/WatchModeManager.h)** / **[`WatchModeManager.cpp`](file:///c:/Work/T-Watch_GPS/WatchModeManager.cpp)**: Mode manager, Swipe UP gesture, and global sleep/wake policy.
- **[`WatchCommandController.h`](file:///c:/Work/T-Watch_GPS/WatchCommandController.h)** / **[`WatchCommandController.cpp`](file:///c:/Work/T-Watch_GPS/WatchCommandController.cpp)**: Command execution controller.
- **[`UartCommandHandler.h`](file:///c:/Work/T-Watch_GPS/UartCommandHandler.h)** / **[`UartCommandHandler.cpp`](file:///c:/Work/T-Watch_GPS/UartCommandHandler.cpp)**: UART serial command input parser.
- **[`HttpServerHandler.h`](file:///c:/Work/T-Watch_GPS/HttpServerHandler.h)** / **[`HttpServerHandler.cpp`](file:///c:/Work/T-Watch_GPS/HttpServerHandler.cpp)**: HTTP web server handler.

---

## 5. Architectural Gotchas & Decisions

1. **NVS Preferences Scope**: Preferences member objects must be scoped locally inside methods (`Preferences prefs; prefs.begin(...) ... prefs.end()`) to avoid ESP32 memory corruption and reboot loops.
2. **WebServer Initialization**: WebServer `begin()` is deferred until Wi-Fi connects to prevent boot crashes.
3. **Compass Hardware**: Dedicated 3-axis magnetometer chip is absent on T-Watch Ultra hardware (`hasCompass` is `false`). Compass mode was completely removed to keep the firmware lean and accurate.
4. **LVGL v9 Gesture Symbol**: Upward swipe gesture in LVGL v9 uses `LV_DIR_TOP`.
