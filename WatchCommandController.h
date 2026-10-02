#ifndef WATCH_COMMAND_CONTROLLER_H
#define WATCH_COMMAND_CONTROLLER_H

#include <Arduino.h>
#include <LilyGoLib.h>
#include <WiFi.h>
#include <Preferences.h>
#include "WatchGlobals.h"

class HttpServerHandler;
class WatchCommandController;

// Command Handler Function Pointer Type
typedef void (WatchCommandController::*CommandHandlerFunc)(const String &args, String &out);

/**
 * @struct CommandEntry
 * @brief Maps a command name to its handler method and help description.
 */
struct CommandEntry {
    const char *name;
    CommandHandlerFunc handler;
    const char *helpText;
};

/**
 * @class WatchCommandController
 * @brief Central command logic controller and persistent NVS configuration manager.
 */
class WatchCommandController {
public:
    WatchCommandController();
    ~WatchCommandController();

    void begin();

    /**
     * @brief Parses and executes a command line string using command table lookup.
     */
    String executeCommand(const String &cmdLine);

    // --- Core Mapped Command Functions ---
    bool setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, String &out);
    bool connectWiFi(const String &arg1, const String &arg2, String &out);
    void scanWiFi(String &out);
    void disconnectWiFi(String &out);
    void setTimeout(uint32_t seconds, String &out);
    bool syncNTP(const String &server, String &out);
    void enableGPS(String &out);
    void disableGPS(String &out);
    void setTimezone(int32_t offsetSecondsOrHours, String &out);
    bool syncGPS(String &out);
    void enableWebserver(String &out);
    void disableWebserver(String &out);
    void setBrightness(uint8_t level, String &out);
    void printHelp(String &out);
    void listWiFiConfigs(String &out);

    // --- Command Table Handlers ---
    void handleHelp(const String &args, String &out);
    void handleSetTime(const String &args, String &out);
    void handleConnectWiFi(const String &args, String &out);
    void handleScanWiFi(const String &args, String &out);
    void handleListWiFi(const String &args, String &out);
    void handleDisconnectWiFi(const String &args, String &out);
    void handleSetTimeout(const String &args, String &out);
    void handleSetBrightness(const String &args, String &out);
    void handleSyncNTP(const String &args, String &out);
    void handleEnableGPS(const String &args, String &out);
    void handleDisableGPS(const String &args, String &out);
    void handleSetTimezone(const String &args, String &out);
    void handleSyncGPS(const String &args, String &out);
    void handleEnableWebserver(const String &args, String &out);
    void handleDisableWebserver(const String &args, String &out);
    void handleSetMode(const String &args, String &out);

    // --- Persistence Helper Methods ---
    bool saveWiFiProfile(const String &name, const String &ssid, const String &password);
    bool loadWiFiProfile(const String &name, String &ssid, String &password);
    bool saveBrightnessConfig(uint8_t level);
    bool loadBrightnessConfig(uint8_t &level);

    bool isGpsEnabled() const { return m_gpsEnabled; }

private:
    int32_t m_timezoneOffsetSec = 0;
    bool m_gpsEnabled = false;
};

#endif // WATCH_COMMAND_CONTROLLER_H
