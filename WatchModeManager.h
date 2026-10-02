#ifndef WATCH_MODE_MANAGER_H
#define WATCH_MODE_MANAGER_H

#include <LilyGoLib.h>
#include <LV_Helper.h>
#include "IWatchView.h"

/**
 * @enum WatchMode
 * @brief Available operational modes for the watch.
 */
enum class WatchMode {
    CLOCK = 0,
    GPS = 1,
    COUNT
};

static const size_t NUM_MODES = static_cast<size_t>(WatchMode::COUNT);

/**
 * @class WatchModeManager
 * @brief Coordinates watch modes polymorphically via IWatchView interface and manages global screen sleep/wake policy.
 */
class WatchModeManager {
public:
    WatchModeManager(uint32_t timeoutMs = 15000);
    ~WatchModeManager();

    void create(IWatchView **views, size_t numViews, lv_obj_t *parent = nullptr);

    void setMode(WatchMode mode);
    WatchMode getMode() const { return m_currentMode; }

    void showModeSwitcher();
    void hideModeSwitcher();

    // --- Global Screen Sleep/Wake Policy ---
    void setTimeout(uint32_t timeoutMs);
    uint32_t getTimeout() const { return m_timeoutMs; }
    void setBrightness(uint8_t level);
    uint8_t getBrightness() const { return m_brightness; }
    void wakeDisplay();
    void sleepDisplay();
    bool isScreenOn() const { return m_isScreenOn; }
    void registerActivity();

private:
    static void gestureCb(lv_event_t *e);
    static void touchCb(lv_event_t *e);
    static void modeBtnCb(lv_event_t *e);
    static void timerCb(lv_timer_t *timer);

    WatchMode m_currentMode = WatchMode::CLOCK;

    lv_obj_t *m_scr = nullptr;

    // Array of registered polymorphic IWatchView pointers
    IWatchView *m_views[NUM_MODES] = {nullptr};

    // Mode Switcher Modal
    lv_obj_t *m_switcherModal = nullptr;

    lv_timer_t *m_timer = nullptr;

    // Screen Power Policy Variables
    uint32_t m_timeoutMs = 15000;
    uint32_t m_lastActivityMs = 0;
    bool m_isScreenOn = true;
    uint8_t m_brightness = DEVICE_MAX_BRIGHTNESS_LEVEL;
};

#endif // WATCH_MODE_MANAGER_H
