#include "GpsView.h"

GpsView::GpsView() {
}

GpsView::~GpsView() {
}

void GpsView::create(lv_obj_t *parent) {
    lv_obj_t *root = parent ? parent : lv_screen_active();

    m_container = lv_obj_create(root);
    lv_obj_set_size(m_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(m_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(m_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(m_container, 0, 0);
    lv_obj_set_style_pad_all(m_container, 10, 0);

    m_titleLabel = lv_label_create(m_container);
    lv_label_set_text(m_titleLabel, "--- GPS NAVIGATION ---");
    lv_obj_set_style_text_color(m_titleLabel, lv_color_hex(0x00FF66), 0);
    lv_obj_set_style_text_align(m_titleLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(m_titleLabel, LV_ALIGN_TOP_MID, 0, 15);

    m_statusLabel = lv_label_create(m_container);
    lv_obj_set_style_text_color(m_statusLabel, lv_color_hex(0x00CC55), 0);
    lv_obj_set_style_text_align(m_statusLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(m_statusLabel, LV_ALIGN_TOP_MID, 0, 45);

    m_satsLabel = lv_label_create(m_container);
    lv_obj_set_style_text_color(m_satsLabel, lv_color_hex(0x00FF66), 0);
    lv_obj_set_style_text_align(m_satsLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(m_satsLabel, LV_ALIGN_TOP_LEFT, 15, 85);

    m_locLabel = lv_label_create(m_container);
    lv_obj_set_style_text_color(m_locLabel, lv_color_hex(0x00FF88), 0);
    lv_obj_set_style_text_align(m_locLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(m_locLabel, LV_ALIGN_TOP_LEFT, 15, 145);

    update();
}

void GpsView::setGpsState(bool enabled) {
    m_gpsEnabled = enabled;
    update();
}

void GpsView::update() {
    if (!m_container) return;

    if (!m_gpsEnabled) {
        lv_label_set_text(m_statusLabel, "STATUS: GPS IS OFF");
        lv_obj_set_style_text_color(m_statusLabel, lv_color_hex(0xFF4444), 0);
        lv_label_set_text(m_satsLabel, "GPS module power is disabled.\n\nUse UART command:\n  'enable gps'\n\nto turn on hardware GPS.");
        lv_label_set_text(m_locLabel, "");
        return;
    }

    lv_label_set_text(m_statusLabel, "STATUS: GPS ONLINE");
    lv_obj_set_style_text_color(m_statusLabel, lv_color_hex(0x00FF66), 0);

    // Poll serial buffer if GPS is enabled
    while (Serial1.available()) {
        char c = Serial1.read();
        instance.gps.encode(c);
    }

    uint32_t trackedSats = instance.gps.satellites.isValid() ? instance.gps.satellites.value() : 0;
    bool hasFix = instance.gps.location.isValid();
    uint32_t fixSats = hasFix ? trackedSats : 0;

    char satsBuf[128];
    snprintf(satsBuf, sizeof(satsBuf),
             "Sattelites Tracked: %u\nSattelites in Fix:   %u\nFix Status:         %s",
             trackedSats, fixSats, hasFix ? "3D LOCK" : "SEARCHING...");
    lv_label_set_text(m_satsLabel, satsBuf);

    if (hasFix) {
        double lat = instance.gps.location.lat();
        double lng = instance.gps.location.lng();
        double alt = instance.gps.altitude.isValid() ? instance.gps.altitude.meters() : 0.0;

        char locBuf[160];
        snprintf(locBuf, sizeof(locBuf),
                 "Current Location:\n  Lat: %.6f°\n  Lon: %.6f°\n  Alt: %.1f m",
                 lat, lng, alt);
        lv_label_set_text(m_locLabel, locBuf);
    } else {
        lv_label_set_text(m_locLabel, "Location:\n  Waiting for satellite fix...");
    }
}
