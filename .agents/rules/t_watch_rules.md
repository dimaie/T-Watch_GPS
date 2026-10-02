# T-Watch_GPS AI Session Guidelines

When working on this workspace, adhere strictly to the following architectural, stability, and documentation rules:

1. **Hardware Scope**:
   - Device: LilyGo T-Watch Ultra (ESP32-S3).
   - Compass hardware (3-axis magnetometer) is absent (`hasCompass` = false). Do NOT re-add compass views or magnetometer code.

2. **Architecture**:
   - UI views implement `IWatchView`. Each view owns its root container `m_container`.
   - `WatchModeManager` operates 100% polymorphically on `IWatchView*` pointers.
   - Screen timeout, inactivity tracking, and backlight sleep/wake are centralized in `WatchModeManager`.
   - Global singletons (`analogClock`, `gpsView`, `modeManager`, `commandController`, `httpHandler`) are declared in `WatchGlobals.h`. Avoid constructor parameter bloat.

3. **Critical Stability Rules**:
   - Scope `Preferences` objects locally inside methods (`Preferences prefs; prefs.begin(...) ... prefs.end()`) to prevent ESP32 NVS memory corruption and boot loops.
   - Defer `WebServer::begin()` until Wi-Fi reports `WL_CONNECTED`.
   - LVGL v9 upward swipe gesture symbol is `LV_DIR_TOP`.

4. **Documentation Maintenance Directive**:
   - AI assistants MUST keep all markdown and documentation files (`AGENTS.md`, `PROJECT_SUMMARY.md`, `.agents/rules/*.md`) up to date whenever features, views, commands, or architectural patterns are modified.
