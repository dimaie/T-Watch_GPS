# Agent Guidelines & Technical Instructions for T-Watch_GPS

This file contains critical behavioral constraints, architectural rules, and hardware gotchas for AI assistants working on the **T-Watch_GPS** project.

---

## 1. Hardware & Capability Rules

- **Target Hardware**: LilyGo T-Watch Ultra (ESP32-S3 microcontroller, ST7789 240x240 display, PCF8563 RTC, U-Blox GPS via `Serial1`, 802.11 b/g/n Wi-Fi radio).
- **GUI Framework**: LVGL v9 initialized via `LV_Helper.h` and `LilyGoLib.h`.
- **Compass / Magnetometer**: **ABSENT on hardware** (`instance.getCapability().hasCompass` is false). Do NOT re-add dedicated compass views, magnetometer routines, or `enable compass` commands.
- **GPS Power & Parsing**: GPS power is toggled via `instance.powerControl(POWER_GPS, state)`. NMEA sentences arrive on `Serial1` and are parsed into `instance.gps` (TinyGPSPlus).

---

## 2. Architecture & Design Patterns

### A. View Polymorphism (`IWatchView`)
- Every watch UI mode MUST implement the **`IWatchView`** interface ([`IWatchView.h`](file:///c:/Work/T-Watch_GPS/IWatchView.h)).
- Each view class creates and owns its root LVGL container (`m_container`).
- `WatchModeManager` operates **exclusively** on `IWatchView*` pointers. Never add concrete view types or view-specific methods (`setGpsState`, `setCompassState`) back into `WatchModeManager`.

### B. Screen Power & Timeout Policy
- Display timeout (`m_timeoutMs`), inactivity tracking (`m_lastActivityMs`), backlight control (`instance.setBrightness()`), and display sleep/wake calls are **centralized inside `WatchModeManager`**.
- Individual views (`AnalogClock`, `GpsView`) MUST NOT manage their own timers or backlight sleep/wake callbacks.

### C. Global Singletons Pattern (`WatchGlobals.h`)
- Main application objects (`analogClock`, `gpsView`, `modeManager`, `commandController`, `httpHandler`) are declared `extern` in **[WatchGlobals.h](file:///c:/Work/T-Watch_GPS/WatchGlobals.h)** and instantiated in **[T-Watch_GPS.ino](file:///c:/Work/T-Watch_GPS/T-Watch_GPS.ino)**.
- Do NOT pass view/manager pointers through `begin()` or `create()` method constructors. Use the global singletons directly.

### D. Command Architecture
- **`WatchCommandController`** is the single source of truth for command execution.
- All command lines pass through `commandController.executeCommand(line)` using `COMMAND_TABLE`.
- Both **`UartCommandHandler`** (Serial input) and **`HttpServerHandler`** (Web Server input) delegate command execution to `commandController`.

---

## 3. Critical Stability Gotchas (DO NOT VIOLATE)

1. **Local NVS `Preferences` Scoping**:
   - `Preferences` objects **MUST** be instantiated locally inside method calls (e.g. `Preferences prefs; prefs.begin("wificfg", ...); ... prefs.end();`).
   - **NEVER** keep `Preferences` objects as class member variables or global instances. Doing so causes ESP32 NVS memory corruption, heap corruption, and infinite watchdog reboot loops.

2. **Deferred WebServer Startup**:
   - `WebServer::begin()` must only be called after Wi-Fi status reaches `WL_CONNECTED`. Calling `m_server.begin()` while Wi-Fi radio is off or disconnected causes ESP32 crash panics.

3. **LVGL v9 Gesture Constants**:
   - In LVGL v9, the upward swipe direction enum constant is **`LV_DIR_TOP`**. `LV_DIR_UP` does not exist in LVGL v9.

4. **UI Design Theme**:
   - Maintain the tactical green-on-black aesthetic across all UI elements (`0x00FF66` primary green, `0x00FF88` secondary green, `0x000000` background, rounded borders).

---

## 4. Documentation Maintenance Directive

- **Mandatory Markdown Updates**: AI assistants MUST keep all non-code documentation files (including [`AGENTS.md`](file:///c:/Work/T-Watch_GPS/AGENTS.md), [`PROJECT_SUMMARY.md`](file:///c:/Work/T-Watch_GPS/PROJECT_SUMMARY.md), and `.agents/rules/*.md`) completely synchronized and updated whenever architectural changes, command modifications, view additions, or feature updates are made.

---

## 5. Key Files Quick Sitemap

- **[`T-Watch_GPS.ino`](file:///c:/Work/T-Watch_GPS/T-Watch_GPS.ino)**: Main sketch entry, singletons definition, hardware `setup()`, and main `loop()`.
- **[`WatchGlobals.h`](file:///c:/Work/T-Watch_GPS/WatchGlobals.h)**: Global singleton header declarations.
- **[`IWatchView.h`](file:///c:/Work/T-Watch_GPS/IWatchView.h)**: Abstract view interface.
- **[`WatchModeManager.h`](file:///c:/Work/T-Watch_GPS/WatchModeManager.h)** / **[`WatchModeManager.cpp`](file:///c:/Work/T-Watch_GPS/WatchModeManager.cpp)**: Mode switching, Swipe UP gesture modal, and global sleep/wake policy.
- **[`WatchCommandController.h`](file:///c:/Work/T-Watch_GPS/WatchCommandController.h)** / **[`WatchCommandController.cpp`](file:///c:/Work/T-Watch_GPS/WatchCommandController.cpp)**: Central command execution matrix.
- **[`AnalogClock.h`](file:///c:/Work/T-Watch_GPS/AnalogClock.h)** / **[`AnalogClock.cpp`](file:///c:/Work/T-Watch_GPS/AnalogClock.cpp)**: Clock UI implementation.
- **[`GpsView.h`](file:///c:/Work/T-Watch_GPS/GpsView.h)** / **[`GpsView.cpp`](file:///c:/Work/T-Watch_GPS/GpsView.cpp)**: GPS status/location UI implementation.
