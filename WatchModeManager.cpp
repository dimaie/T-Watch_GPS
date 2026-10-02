#include "WatchModeManager.h"

WatchModeManager::WatchModeManager(uint32_t timeoutMs)
    : m_timeoutMs(timeoutMs), m_lastActivityMs(0), m_isScreenOn(true) {
}

WatchModeManager::~WatchModeManager() {
    if (m_timer) {
        lv_timer_delete(m_timer);
        m_timer = nullptr;
    }
}

void WatchModeManager::setTimeout(uint32_t timeoutMs) {
    m_timeoutMs = timeoutMs;
    m_lastActivityMs = millis();
}

void WatchModeManager::setBrightness(uint8_t level) {
    m_brightness = level;
    if (m_isScreenOn) {
        instance.setBrightness(m_brightness);
    }
}

void WatchModeManager::registerActivity() {
    m_lastActivityMs = millis();
    if (!m_isScreenOn) {
        wakeDisplay();
    }
}

void WatchModeManager::wakeDisplay() {
    instance.wakeupDisplay();
    instance.setBrightness(m_brightness);
    m_isScreenOn = true;
    m_lastActivityMs = millis();

    // Force refresh active mode view on wake
    size_t activeIdx = static_cast<size_t>(m_currentMode);
    if (activeIdx < NUM_MODES && m_views[activeIdx]) {
        m_views[activeIdx]->update();
    }
}

void WatchModeManager::sleepDisplay() {
    instance.setBrightness(0);
    instance.sleepDisplay();
    m_isScreenOn = false;
}

void WatchModeManager::create(IWatchView **views, size_t numViews, lv_obj_t *parent) {
    m_scr = parent ? parent : lv_screen_active();
    lv_obj_set_style_bg_color(m_scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(m_scr, LV_OPA_COVER, 0);

    lv_obj_add_flag(m_scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(m_scr, touchCb, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(m_scr, gestureCb, LV_EVENT_GESTURE, this);

    size_t count = (numViews < NUM_MODES) ? numViews : NUM_MODES;
    for (size_t i = 0; i < count; i++) {
        m_views[i] = views[i];
        IWatchView *v = m_views[i];
        if (v) {
            if (!v->getContainer()) {
                v->create(m_scr);
            }
            if (v->getContainer()) {
                lv_obj_add_flag(v->getContainer(), LV_OBJ_FLAG_CLICKABLE);
                lv_obj_add_event_cb(v->getContainer(), touchCb, LV_EVENT_PRESSED, this);
                lv_obj_add_event_cb(v->getContainer(), gestureCb, LV_EVENT_GESTURE, this);
            }
        }
    }

    // Build Mode Switcher Modal dynamically from polymorphic IWatchView list
    m_switcherModal = lv_obj_create(m_scr);
    lv_obj_set_size(m_switcherModal, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(m_switcherModal, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(m_switcherModal, LV_OPA_80, 0);
    lv_obj_set_style_border_width(m_switcherModal, 0, 0);
    lv_obj_add_flag(m_switcherModal, LV_OBJ_FLAG_HIDDEN);

    // Modal Card
    lv_obj_t *card = lv_obj_create(m_switcherModal);
    lv_obj_set_size(card, 210, 200);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x051505), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0x00FF66), 0);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_pad_all(card, 10, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *modalTitle = lv_label_create(card);
    lv_label_set_text(modalTitle, "SELECT MODE");
    lv_obj_set_style_text_color(modalTitle, lv_color_hex(0x00FF66), 0);

    for (size_t i = 0; i < NUM_MODES; i++) {
        if (m_views[i]) {
            lv_obj_t *btn = lv_button_create(card);
            lv_obj_set_size(btn, 170, 36);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x002200), 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x00FF66), 0);
            lv_obj_set_style_border_width(btn, 1, 0);
            lv_obj_set_style_radius(btn, 6, 0);
            lv_obj_add_event_cb(btn, modeBtnCb, LV_EVENT_CLICKED, this);

            intptr_t modeVal = (intptr_t)i;
            lv_obj_set_user_data(btn, (void *)modeVal);

            lv_obj_t *lbl = lv_label_create(btn);
            lv_label_set_text_fmt(lbl, "%d. %s", (int)(i + 1), m_views[i]->getModeName());
            lv_obj_set_style_text_color(lbl, lv_color_hex(0x00FF66), 0);
            lv_obj_center(lbl);
        }
    }

    m_lastActivityMs = millis();
    m_isScreenOn = true;

    setMode(WatchMode::CLOCK);

    m_timer = lv_timer_create(timerCb, 500, this);
}


void WatchModeManager::setMode(WatchMode mode) {
    m_currentMode = mode;
    registerActivity();

    // Polymorphically hide containers of all registered views
    for (size_t i = 0; i < NUM_MODES; i++) {
        if (m_views[i] && m_views[i]->getContainer()) {
            lv_obj_add_flag(m_views[i]->getContainer(), LV_OBJ_FLAG_HIDDEN);
        }
    }

    // Show container of target active view
    size_t activeIdx = static_cast<size_t>(m_currentMode);
    if (activeIdx < NUM_MODES && m_views[activeIdx]) {
        IWatchView *activeView = m_views[activeIdx];
        if (activeView->getContainer()) {
            lv_obj_clear_flag(activeView->getContainer(), LV_OBJ_FLAG_HIDDEN);
            activeView->update();
        }
    }
}

void WatchModeManager::showModeSwitcher() {
    registerActivity();
    if (m_switcherModal) {
        lv_obj_clear_flag(m_switcherModal, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(m_switcherModal);
    }
}

void WatchModeManager::hideModeSwitcher() {
    registerActivity();
    if (m_switcherModal) {
        lv_obj_add_flag(m_switcherModal, LV_OBJ_FLAG_HIDDEN);
    }
}

void WatchModeManager::touchCb(lv_event_t *e) {
    WatchModeManager *mgr = static_cast<WatchModeManager *>(lv_event_get_user_data(e));
    if (mgr) {
        mgr->registerActivity();
    }
}

void WatchModeManager::gestureCb(lv_event_t *e) {
    WatchModeManager *mgr = static_cast<WatchModeManager *>(lv_event_get_user_data(e));
    if (!mgr) return;

    mgr->registerActivity();

    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_TOP) {
        mgr->showModeSwitcher();
    }
}

void WatchModeManager::modeBtnCb(lv_event_t *e) {
    WatchModeManager *mgr = static_cast<WatchModeManager *>(lv_event_get_user_data(e));
    if (!mgr) return;

    mgr->registerActivity();

    lv_obj_t *btn = lv_event_get_target_obj(e);
    intptr_t modeVal = (intptr_t)lv_obj_get_user_data(btn);
    WatchMode selectedMode = static_cast<WatchMode>(modeVal);

    mgr->setMode(selectedMode);
    mgr->hideModeSwitcher();
}

void WatchModeManager::timerCb(lv_timer_t *timer) {
    WatchModeManager *mgr = static_cast<WatchModeManager *>(lv_timer_get_user_data(timer));
    if (!mgr) return;

    uint32_t now = millis();

    // Global Sleep/Wake Policy check across all modes
    if (mgr->m_isScreenOn && mgr->m_timeoutMs > 0 && (now - mgr->m_lastActivityMs >= mgr->m_timeoutMs)) {
        mgr->sleepDisplay();
        return;
    }

    // Do not refresh GUI elements if screen is sleeping
    if (!mgr->m_isScreenOn) {
        return;
    }

    size_t activeIdx = static_cast<size_t>(mgr->m_currentMode);
    if (activeIdx < NUM_MODES && mgr->m_views[activeIdx]) {
        mgr->m_views[activeIdx]->update();
    }
}
