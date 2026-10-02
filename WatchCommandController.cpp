#include "WatchCommandController.h"
#include "AnalogClock.h"
#include "GpsView.h"
#include "WatchModeManager.h"
#include "HttpServerHandler.h"
#include <time.h>

// Static Command Table Mapping Commands to Handler Methods
static const CommandEntry COMMAND_TABLE[] = {
    {"help",              &WatchCommandController::handleHelp,             "Show command list"},
    {"set time",          &WatchCommandController::handleSetTime,          "Set RTC date & time (YYYY-MM-DD HH:MM:SS)"},
    {"connect wifi",      &WatchCommandController::handleConnectWiFi,      "Connect to Wi-Fi [SSID/CONFIG] [PASS]"},
    {"scan wifi",         &WatchCommandController::handleScanWiFi,         "Scan nearby Wi-Fi networks"},
    {"list wifi",         &WatchCommandController::handleListWiFi,         "List all saved Wi-Fi configurations"},
    {"disconnect wifi",   &WatchCommandController::handleDisconnectWiFi,   "Disconnect Wi-Fi and power off radio"},
    {"set timeout",       &WatchCommandController::handleSetTimeout,       "Set display sleep timeout in seconds"},
    {"set brightness",    &WatchCommandController::handleSetBrightness,    "Set display brightness [0-255 or 0-100%]"},
    {"brightness",        &WatchCommandController::handleSetBrightness,    "Set display brightness [0-255 or 0-100%]"},
    {"sync ntp",          &WatchCommandController::handleSyncNTP,          "Sync RTC time via NTP [server]"},
    {"enable gps",        &WatchCommandController::handleEnableGPS,        "Power on GPS module"},
    {"disable gps",       &WatchCommandController::handleDisableGPS,       "Power off GPS module"},
    {"set timezone",      &WatchCommandController::handleSetTimezone,      "Set timezone offset in hours or seconds"},
    {"sync gps",          &WatchCommandController::handleSyncGPS,          "Sync RTC time from GPS satellites"},
    {"enable webserver",  &WatchCommandController::handleEnableWebserver,  "Enable and start HTTP web server"},
    {"disable webserver", &WatchCommandController::handleDisableWebserver, "Disable and stop HTTP web server"},
    {"set mode",          &WatchCommandController::handleSetMode,          "Set watch operational mode [clock|gps]"},
    {"mode",              &WatchCommandController::handleSetMode,          "Set watch operational mode [clock|gps]"}
};

static const size_t NUM_COMMANDS = sizeof(COMMAND_TABLE) / sizeof(COMMAND_TABLE[0]);

WatchCommandController::WatchCommandController() {
}

WatchCommandController::~WatchCommandController() {
}

void WatchCommandController::begin() {
    uint8_t savedBrightness = DEVICE_MAX_BRIGHTNESS_LEVEL;
    if (loadBrightnessConfig(savedBrightness)) {
        modeManager.setBrightness(savedBrightness);
    }
}

bool WatchCommandController::saveWiFiProfile(const String &name, const String &ssid, const String &password) {
    if (name.length() == 0 || ssid.length() == 0) return false;
    Preferences prefs;
    if (!prefs.begin("wificfg", false)) return false;

    String profilesStr = prefs.getString("proflist", "");
    if (profilesStr.indexOf(name) == -1) {
        if (profilesStr.length() > 0) profilesStr += ",";
        profilesStr += name;
        prefs.putString("proflist", profilesStr);
    }

    prefs.putString((name + "_s").c_str(), ssid);
    prefs.putString((name + "_p").c_str(), password);
    prefs.putString("last", name);
    prefs.end();
    return true;
}

bool WatchCommandController::loadWiFiProfile(const String &name, String &ssid, String &password) {
    if (name.length() == 0) return false;
    Preferences prefs;
    if (!prefs.begin("wificfg", true)) return false;

    String keyS = name + "_s";
    String keyP = name + "_p";
    if (!prefs.isKey(keyS.c_str())) {
        prefs.end();
        return false;
    }
    ssid = prefs.getString(keyS.c_str(), "");
    password = prefs.getString(keyP.c_str(), "");
    prefs.end();
    return (ssid.length() > 0);
}

void WatchCommandController::listWiFiConfigs(String &out) {
    Preferences prefs;
    if (!prefs.begin("wificfg", true)) {
        out += "[CONFIG] No saved Wi-Fi profiles in NVS storage.\r\n";
        return;
    }

    String last = prefs.getString("last", "<none>");
    String profilesStr = prefs.getString("proflist", "");
    prefs.end();

    out += "\r\n--- Saved Wi-Fi Configurations ---\r\n";
    out += "  Default/Last Profile: '" + last + "'\r\n";

    if (profilesStr.length() == 0) {
        out += "  No saved profiles in NVS storage.\r\n\r\n";
        return;
    }

    out += "  Saved Profiles:\r\n";
    int start = 0;
    int idx = 0;
    while (start < (int)profilesStr.length()) {
        int commaIdx = profilesStr.indexOf(',', start);
        if (commaIdx == -1) commaIdx = profilesStr.length();
        String pname = profilesStr.substring(start, commaIdx);
        pname.trim();
        if (pname.length() > 0) {
            String ssid, pass;
            if (loadWiFiProfile(pname, ssid, pass)) {
                idx++;
                char buf[128];
                snprintf(buf, sizeof(buf), "    %2d: Profile '%s' -> SSID: '%s'\r\n", idx, pname.c_str(), ssid.c_str());
                out += buf;
            }
        }
        start = commaIdx + 1;
    }
    out += "\r\n";
}

bool WatchCommandController::saveBrightnessConfig(uint8_t level) {
    Preferences prefs;
    if (!prefs.begin("watchcfg", false)) return false;
    prefs.putUChar("bright", level);
    prefs.end();
    return true;
}

bool WatchCommandController::loadBrightnessConfig(uint8_t &level) {
    Preferences prefs;
    if (!prefs.begin("watchcfg", true)) return false;
    if (!prefs.isKey("bright")) {
        prefs.end();
        return false;
    }
    level = prefs.getUChar("bright", DEVICE_MAX_BRIGHTNESS_LEVEL);
    prefs.end();
    return true;
}

bool WatchCommandController::setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, String &out) {
    RTC_DateTime dt(year, month, day, hour, minute, second);
    instance.rtc.setDateTime(dt);
    analogClock.updateTime();
    char buf[128];
    snprintf(buf, sizeof(buf), "[CMD] Time successfully set to %04d-%02d-%02d %02d:%02d:%02d\r\n",
             year, month, day, hour, minute, second);
    out += buf;
    return true;
}

bool WatchCommandController::connectWiFi(const String &arg1, const String &arg2, String &out) {
    String targetSSID = "";
    String targetPass = "";
    String profileName = "";
    bool isFromFlash = false;

    if (arg1.length() == 0) {
        Preferences prefs;
        if (prefs.begin("wificfg", true)) {
            profileName = prefs.getString("last", "");
            prefs.end();
        }

        if (profileName.length() == 0 || !loadWiFiProfile(profileName, targetSSID, targetPass)) {
            out += "[CMD] Error: No saved Wi-Fi configuration found. Provide SSID & password.\r\n";
            return false;
        }
        isFromFlash = true;
        out += "[CMD] Using saved profile '" + profileName + "'...\r\n";
    } else if (arg2.length() == 0) {
        if (loadWiFiProfile(arg1, targetSSID, targetPass)) {
            profileName = arg1;
            isFromFlash = true;
            out += "[CMD] Loaded saved profile '" + profileName + "' (SSID: " + targetSSID + ")...\r\n";
        } else {
            targetSSID = arg1;
            targetPass = "";
            profileName = arg1;
            isFromFlash = false;
        }
    } else {
        targetSSID = arg1;
        targetPass = arg2;
        profileName = arg1;
        isFromFlash = false;
    }

    out += "[CMD] Connecting to Wi-Fi SSID: '" + targetSSID + "'...\r\n";
    WiFi.mode(WIFI_STA);
    WiFi.begin(targetSSID.c_str(), targetPass.c_str());

    uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startMs < 10000)) {
        delay(500);
        out += ".";
    }
    out += "\r\n";

    if (WiFi.status() == WL_CONNECTED) {
        out += "[CMD] Wi-Fi Connected! IP: " + WiFi.localIP().toString() + "\r\n";
        if (!isFromFlash) {
            saveWiFiProfile(profileName, targetSSID, targetPass);
            out += "[CMD] Saved Wi-Fi config '" + profileName + "' to NVS storage.\r\n";
        }
        return true;
    } else {
        out += "[CMD] Wi-Fi connection failed or timed out.\r\n";
        return false;
    }
}

void WatchCommandController::scanWiFi(String &out) {
    out += "[CMD] Scanning Wi-Fi networks...\r\n";
    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks();
    if (n == 0) {
        out += "[CMD] No networks found.\r\n";
    } else {
        char buf[128];
        snprintf(buf, sizeof(buf), "[CMD] %d networks found:\r\n", n);
        out += buf;
        for (int i = 0; i < n; ++i) {
            snprintf(buf, sizeof(buf), "  %2d: %-24s (%d dBm) %s\r\n",
                     i + 1,
                     WiFi.SSID(i).c_str(),
                     WiFi.RSSI(i),
                     (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "[Open]" : "[Encrypted]");
            out += buf;
        }
    }
}

void WatchCommandController::disconnectWiFi(String &out) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    out += "[CMD] Wi-Fi disconnected and powered off.\r\n";
}

void WatchCommandController::setTimeout(uint32_t seconds, String &out) {
    modeManager.setTimeout(seconds * 1000);
    char buf[64];
    snprintf(buf, sizeof(buf), "[CMD] Display timeout set to %u seconds.\r\n", seconds);
    out += buf;
}

void WatchCommandController::setBrightness(uint8_t level, String &out) {
    modeManager.setBrightness(level);
    saveBrightnessConfig(level);

    uint8_t pct = (uint8_t)(((uint32_t)level * 100 + 127) / 255);
    char buf[128];
    snprintf(buf, sizeof(buf), "[CMD] Display brightness set to %u (%u%%) and saved to NVS.\r\n", level, pct);
    out += buf;
}

bool WatchCommandController::syncNTP(const String &server, String &out) {
    if (WiFi.status() != WL_CONNECTED) {
        out += "[CMD] Error: Wi-Fi is not connected. Connect to Wi-Fi first.\r\n";
        return false;
    }

    out += "[CMD] Syncing time with NTP server '" + server + "'...\r\n";
    configTime(m_timezoneOffsetSec, 0, server.c_str());

    struct tm timeinfo;
    uint32_t startMs = millis();
    bool timeAcquired = false;

    while (millis() - startMs < 10000) {
        if (getLocalTime(&timeinfo)) {
            timeAcquired = true;
            break;
        }
        delay(500);
    }

    if (timeAcquired) {
        RTC_DateTime dt(timeinfo);
        instance.rtc.setDateTime(dt);
        analogClock.updateTime();
        char buf[128];
        snprintf(buf, sizeof(buf), "[CMD] NTP Time Sync successful! Time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
                 dt.getYear(), dt.getMonth(), dt.getDay(),
                 dt.getHour(), dt.getMinute(), dt.getSecond());
        out += buf;
        return true;
    } else {
        out += "[CMD] NTP Time Sync failed (timeout).\r\n";
        return false;
    }
}

void WatchCommandController::enableGPS(String &out) {
    instance.powerControl(POWER_GPS, true);
    instance.initGPS();
    m_gpsEnabled = true;
    gpsView.setGpsState(true);
    out += "[CMD] GPS module powered ON and initialized.\r\n";
}

void WatchCommandController::disableGPS(String &out) {
    instance.powerControl(POWER_GPS, false);
    m_gpsEnabled = false;
    gpsView.setGpsState(false);
    out += "[CMD] GPS module powered OFF.\r\n";
}

void WatchCommandController::setTimezone(int32_t offsetSecondsOrHours, String &out) {
    if (offsetSecondsOrHours >= -14 && offsetSecondsOrHours <= 14) {
        m_timezoneOffsetSec = offsetSecondsOrHours * 3600;
    } else {
        m_timezoneOffsetSec = offsetSecondsOrHours;
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "[CMD] Timezone offset set to %d seconds (%d hours).\r\n",
             m_timezoneOffsetSec, m_timezoneOffsetSec / 3600);
    out += buf;
}

bool WatchCommandController::syncGPS(String &out) {
    if (!m_gpsEnabled) {
        out += "[CMD] Enabling GPS for sync...\r\n";
        enableGPS(out);
    }

    out += "[CMD] Waiting for GPS lock (up to 15 seconds)...\r\n";
    uint32_t startMs = millis();
    bool fixAcquired = false;

    while (millis() - startMs < 15000) {
        if (m_gpsEnabled) {
            while (Serial1.available()) {
                char c = Serial1.read();
                instance.gps.encode(c);
            }
        }

        if (instance.gps.date.isValid() && instance.gps.time.isValid() && instance.gps.date.year() >= 2024) {
            fixAcquired = true;
            break;
        }
        delay(100);
    }

    if (fixAcquired) {
        uint16_t yr = instance.gps.date.year();
        uint8_t mo = instance.gps.date.month();
        uint8_t dy = instance.gps.date.day();
        uint8_t hr = instance.gps.time.hour();
        uint8_t mn = instance.gps.time.minute();
        uint8_t sc = instance.gps.time.second();

        struct tm t_utc;
        memset(&t_utc, 0, sizeof(t_utc));
        t_utc.tm_year = yr - 1900;
        t_utc.tm_mon = mo - 1;
        t_utc.tm_mday = dy;
        t_utc.tm_hour = hr;
        t_utc.tm_min = mn;
        t_utc.tm_sec = sc;

        time_t utc_epoch = mktime(&t_utc);
        time_t local_epoch = utc_epoch + m_timezoneOffsetSec;
        struct tm *t_local = localtime(&local_epoch);

        RTC_DateTime dt(*t_local);
        instance.rtc.setDateTime(dt);
        analogClock.updateTime();

        char buf[128];
        snprintf(buf, sizeof(buf), "[CMD] GPS Sync Success! Local Time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
                 dt.getYear(), dt.getMonth(), dt.getDay(),
                 dt.getHour(), dt.getMinute(), dt.getSecond());
        out += buf;
        return true;
    } else {
        out += "[CMD] GPS Sync failed: Valid satellite fix not acquired.\r\n";
        return false;
    }
}

void WatchCommandController::enableWebserver(String &out) {
    httpHandler.start();
    out += "[CMD] WebServer enabled.\r\n";
}

void WatchCommandController::disableWebserver(String &out) {
    httpHandler.stop();
    out += "[CMD] WebServer disabled and stopped.\r\n";
}

void WatchCommandController::printHelp(String &out) {
    out += "\r\n--- Available Watch Commands ---\r\n";
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        char buf[128];
        snprintf(buf, sizeof(buf), "  %-32s : %s\r\n", COMMAND_TABLE[i].name, COMMAND_TABLE[i].helpText);
        out += buf;
    }
    out += "\r\n";
}

// --- Command Table Delegate Handlers ---
void WatchCommandController::handleHelp(const String &args, String &out) {
    printHelp(out);
}

void WatchCommandController::handleSetTime(const String &args, String &out) {
    int yr, mo, dy, hr, mn, sc;
    if (sscanf(args.c_str(), "%d-%d-%d %d:%d:%d", &yr, &mo, &dy, &hr, &mn, &sc) == 6 ||
        sscanf(args.c_str(), "%d %d %d %d %d %d", &yr, &mo, &dy, &hr, &mn, &sc) == 6) {
        setTime((uint16_t)yr, (uint8_t)mo, (uint8_t)dy, (uint8_t)hr, (uint8_t)mn, (uint8_t)sc, out);
    } else {
        out += "[CMD] Invalid set time format! Use: set time YYYY-MM-DD HH:MM:SS\r\n";
    }
}

void WatchCommandController::handleConnectWiFi(const String &args, String &out) {
    int spaceIdx = args.indexOf(' ');
    String arg1 = (spaceIdx != -1) ? args.substring(0, spaceIdx) : args;
    String arg2 = (spaceIdx != -1) ? args.substring(spaceIdx + 1) : "";
    arg1.trim();
    arg2.trim();
    connectWiFi(arg1, arg2, out);
}

void WatchCommandController::handleScanWiFi(const String &args, String &out) {
    scanWiFi(out);
}

void WatchCommandController::handleListWiFi(const String &args, String &out) {
    listWiFiConfigs(out);
}

void WatchCommandController::handleDisconnectWiFi(const String &args, String &out) {
    disconnectWiFi(out);
}

void WatchCommandController::handleSetTimeout(const String &args, String &out) {
    uint32_t secs = (uint32_t)args.toInt();
    setTimeout(secs, out);
}

void WatchCommandController::handleSetBrightness(const String &args, String &out) {
    String arg = args;
    arg.trim();
    if (arg.length() == 0) {
        uint8_t cur = modeManager.getBrightness();
        uint8_t pct = (uint8_t)(((uint32_t)cur * 100 + 127) / 255);
        char buf[128];
        snprintf(buf, sizeof(buf), "[CMD] Current display brightness: %u/255 (%u%%). Usage: set brightness <0-255|0-100%%>\r\n", cur, pct);
        out += buf;
        return;
    }

    int level = -1;
    if (arg.endsWith("%")) {
        String pctStr = arg.substring(0, arg.length() - 1);
        pctStr.trim();
        if (pctStr.length() > 0) {
            bool valid = true;
            for (size_t i = 0; i < pctStr.length(); i++) {
                if (!isdigit(pctStr.charAt(i))) { valid = false; break; }
            }
            if (valid) {
                int pct = pctStr.toInt();
                if (pct < 0) pct = 0;
                if (pct > 100) pct = 100;
                level = (pct * 255 + 50) / 100;
            }
        }
    } else {
        bool valid = true;
        for (size_t i = 0; i < arg.length(); i++) {
            if (!isdigit(arg.charAt(i))) { valid = false; break; }
        }
        if (valid) {
            level = arg.toInt();
            if (level < 0) level = 0;
            if (level > 255) level = 255;
        }
    }

    if (level < 0) {
        out += "[CMD] Invalid brightness value! Usage: set brightness <0-255|0-100%>\r\n";
        return;
    }

    setBrightness((uint8_t)level, out);
}

void WatchCommandController::handleSyncNTP(const String &args, String &out) {
    String server = args;
    server.trim();
    if (server.length() == 0) server = "pool.ntp.org";
    syncNTP(server, out);
}

void WatchCommandController::handleEnableGPS(const String &args, String &out) {
    enableGPS(out);
}

void WatchCommandController::handleDisableGPS(const String &args, String &out) {
    disableGPS(out);
}

void WatchCommandController::handleSetTimezone(const String &args, String &out) {
    int32_t tz = (int32_t)args.toInt();
    setTimezone(tz, out);
}

void WatchCommandController::handleSyncGPS(const String &args, String &out) {
    syncGPS(out);
}

void WatchCommandController::handleEnableWebserver(const String &args, String &out) {
    enableWebserver(out);
}

void WatchCommandController::handleDisableWebserver(const String &args, String &out) {
    disableWebserver(out);
}

void WatchCommandController::handleSetMode(const String &args, String &out) {
    String m = args;
    m.trim();
    m.toLowerCase();

    if (m == "clock") {
        modeManager.setMode(WatchMode::CLOCK);
        out += "[CMD] Watch mode switched to CLOCK.\r\n";
    } else if (m == "gps") {
        modeManager.setMode(WatchMode::GPS);
        out += "[CMD] Watch mode switched to GPS.\r\n";
    } else {
        out += "[CMD] Invalid mode! Usage: set mode <clock|gps>\r\n";
    }
}

String WatchCommandController::executeCommand(const String &cmdLine) {
    String line = cmdLine;
    line.trim();
    if (line.length() == 0) return "";

    String lowerLine = line;
    lowerLine.toLowerCase();
    String out = "";

    // Array-based command lookup
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        const char *cmdName = COMMAND_TABLE[i].name;
        size_t cmdLen = strlen(cmdName);

        // Match exact command OR prefix command followed by space/end of string
        if (lowerLine == cmdName ||
           (lowerLine.startsWith(cmdName) && (lowerLine.length() == cmdLen || lowerLine.charAt(cmdLen) == ' '))) {

            String args = line.substring(cmdLen);
            args.trim();
            if (COMMAND_TABLE[i].handler != nullptr) {
                (this->*COMMAND_TABLE[i].handler)(args, out);
            }
            return out;
        }
    }

    out += "[CMD] Unknown command: '" + line + "'. Type 'help' for available commands.\r\n";
    return out;
}
