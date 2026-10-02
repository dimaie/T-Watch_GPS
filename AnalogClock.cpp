#include "AnalogClock.h"

AnalogClock::AnalogClock() {
}

AnalogClock::~AnalogClock() {
}

void AnalogClock::create(lv_obj_t *parent) {
    lv_obj_t *root = parent ? parent : lv_screen_active();

    m_container = lv_obj_create(root);
    lv_obj_set_size(m_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(m_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(m_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(m_container, 0, 0);
    lv_obj_set_style_pad_all(m_container, 0, 0);

    int32_t scr_w = instance.width();
    int32_t scr_h = instance.height();
    if (scr_w <= 0) scr_w = 240;
    if (scr_h <= 0) scr_h = 240;

    m_cx = scr_w / 2;
    m_cy = scr_h / 2;
    m_dialR = (scr_w < scr_h ? scr_w : scr_h) / 2 - 15;

    // Outer Dial Ring
    m_dialRing = lv_arc_create(m_container);
    lv_arc_set_bg_angles(m_dialRing, 0, 360);
    lv_arc_set_angles(m_dialRing, 0, 360);
    lv_obj_set_size(m_dialRing, m_dialR * 2, m_dialR * 2);
    lv_obj_center(m_dialRing);
    lv_obj_remove_style(m_dialRing, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(m_dialRing, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(m_dialRing, lv_color_hex(0x004411), LV_PART_MAIN);
    lv_obj_set_style_arc_color(m_dialRing, lv_color_hex(0x00FF66), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(m_dialRing, 3, LV_PART_MAIN);
    lv_obj_set_style_arc_width(m_dialRing, 3, LV_PART_INDICATOR);

    // Minute and Hour Tick Marks
    for (int i = 0; i < 60; i++) {
        double angle = (i * 6.0 - 90.0) * M_PI / 180.0;
        bool is_hour = (i % 5 == 0);

        int32_t r_outer = m_dialR - 4;
        int32_t r_inner = is_hour ? (m_dialR - 20) : (m_dialR - 12);

        m_minTickPts[i][0].x = m_cx + (int32_t)(r_inner * cos(angle));
        m_minTickPts[i][0].y = m_cy + (int32_t)(r_inner * sin(angle));
        m_minTickPts[i][1].x = m_cx + (int32_t)(r_outer * cos(angle));
        m_minTickPts[i][1].y = m_cy + (int32_t)(r_outer * sin(angle));

        lv_obj_t *tick = lv_line_create(m_container);
        lv_line_set_points(tick, m_minTickPts[i], 2);
        lv_obj_clear_flag(tick, LV_OBJ_FLAG_CLICKABLE);
        if (is_hour) {
            lv_obj_set_style_line_width(tick, 4, 0);
            lv_obj_set_style_line_color(tick, lv_color_hex(0x00FF66), 0);
        } else {
            lv_obj_set_style_line_width(tick, 2, 0);
            lv_obj_set_style_line_color(tick, lv_color_hex(0x006622), 0);
        }
        lv_obj_set_style_line_rounded(tick, true, 0);
    }

    // Hour Numerals (12, 1, 2, ..., 11)
    for (int i = 0; i < 12; i++) {
        double angle = (i * 30.0 - 90.0) * M_PI / 180.0;
        int32_t r_text = m_dialR - 34;

        int32_t tx = m_cx + (int32_t)(r_text * cos(angle));
        int32_t ty = m_cy + (int32_t)(r_text * sin(angle));

        int num = (i == 0) ? 12 : i;

        lv_obj_t *num_label = lv_label_create(m_container);
        lv_label_set_text_fmt(num_label, "%d", num);
        lv_obj_set_style_text_color(num_label, lv_color_hex(0x00FF66), 0);
        lv_obj_clear_flag(num_label, LV_OBJ_FLAG_CLICKABLE);

        // Center position offset for font
        lv_obj_set_pos(num_label, tx - 10, ty - 10);
    }

    // Date Label (Green on Black)
    m_dateLabel = lv_label_create(m_container);
    lv_obj_set_style_text_color(m_dateLabel, lv_color_hex(0x00CC55), 0);
    lv_obj_set_size(m_dateLabel, 140, 30);
    lv_obj_set_style_text_align(m_dateLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(m_dateLabel, m_cx - 70, m_cy + (m_dialR / 2) - 10);
    lv_obj_clear_flag(m_dateLabel, LV_OBJ_FLAG_CLICKABLE);

    // Clock Hands (Hour, Minute, Second)
    m_hourLine = lv_line_create(m_container);
    lv_obj_set_style_line_width(m_hourLine, 7, 0);
    lv_obj_set_style_line_color(m_hourLine, lv_color_hex(0x00FF66), 0);
    lv_obj_set_style_line_rounded(m_hourLine, true, 0);
    lv_obj_clear_flag(m_hourLine, LV_OBJ_FLAG_CLICKABLE);

    m_minLine = lv_line_create(m_container);
    lv_obj_set_style_line_width(m_minLine, 5, 0);
    lv_obj_set_style_line_color(m_minLine, lv_color_hex(0x00FF88), 0);
    lv_obj_set_style_line_rounded(m_minLine, true, 0);
    lv_obj_clear_flag(m_minLine, LV_OBJ_FLAG_CLICKABLE);

    m_secLine = lv_line_create(m_container);
    lv_obj_set_style_line_width(m_secLine, 2, 0);
    lv_obj_set_style_line_color(m_secLine, lv_color_hex(0x39FF14), 0);
    lv_obj_set_style_line_rounded(m_secLine, true, 0);
    lv_obj_clear_flag(m_secLine, LV_OBJ_FLAG_CLICKABLE);

    // Center Pivot Cap
    m_centerCap = lv_obj_create(m_container);
    lv_obj_set_size(m_centerCap, 14, 14);
    lv_obj_set_style_radius(m_centerCap, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(m_centerCap, lv_color_hex(0x00FF66), 0);
    lv_obj_set_style_bg_opa(m_centerCap, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(m_centerCap, lv_color_hex(0x003311), 0);
    lv_obj_set_style_border_width(m_centerCap, 2, 0);
    lv_obj_set_pos(m_centerCap, m_cx - 7, m_cy - 7);
    lv_obj_clear_flag(m_centerCap, LV_OBJ_FLAG_CLICKABLE);

    updateTime();
}

void AnalogClock::updateTime() {
    RTC_DateTime dt = instance.rtc.getDateTime();

    // If RTC year is uninitialized/invalid, set it to compilation time
    if (dt.getYear() < 2024) {
        instance.rtc.setDateTime(RTC_DateTime(__DATE__, __TIME__));
        dt = instance.rtc.getDateTime();
    }

    // Angles for watch hands in degrees (-90deg aligns 0deg to 12 o'clock)
    double h_deg = ((dt.getHour() % 12) + dt.getMinute() / 60.0 + dt.getSecond() / 3600.0) * 30.0 - 90.0;
    double m_deg = (dt.getMinute() + dt.getSecond() / 60.0) * 6.0 - 90.0;
    double s_deg = dt.getSecond() * 6.0 - 90.0;

    double h_rad = h_deg * M_PI / 180.0;
    double m_rad = m_deg * M_PI / 180.0;
    double s_rad = s_deg * M_PI / 180.0;

    int32_t h_len = (int32_t)(m_dialR * 0.50);
    int32_t m_len = (int32_t)(m_dialR * 0.74);
    int32_t s_len = (int32_t)(m_dialR * 0.85);

    int32_t h_back = 12;
    int32_t m_back = 16;
    int32_t s_back = 22;

    m_hourPts[0].x = m_cx - (int32_t)(h_back * cos(h_rad));
    m_hourPts[0].y = m_cy - (int32_t)(h_back * sin(h_rad));
    m_hourPts[1].x = m_cx + (int32_t)(h_len * cos(h_rad));
    m_hourPts[1].y = m_cy + (int32_t)(h_len * sin(h_rad));

    m_minPts[0].x = m_cx - (int32_t)(m_back * cos(m_rad));
    m_minPts[0].y = m_cy - (int32_t)(m_back * sin(m_rad));
    m_minPts[1].x = m_cx + (int32_t)(m_len * cos(m_rad));
    m_minPts[1].y = m_cy + (int32_t)(m_len * sin(m_rad));

    m_secPts[0].x = m_cx - (int32_t)(s_back * cos(s_rad));
    m_secPts[0].y = m_cy - (int32_t)(s_back * sin(s_rad));
    m_secPts[1].x = m_cx + (int32_t)(s_len * cos(s_rad));
    m_secPts[1].y = m_cy + (int32_t)(s_len * sin(s_rad));

    lv_line_set_points(m_hourLine, m_hourPts, 2);
    lv_line_set_points(m_minLine, m_minPts, 2);
    lv_line_set_points(m_secLine, m_secPts, 2);

    static const char *days_str[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    static const char *months_str[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    uint8_t w = dt.getWeek() % 7;
    uint8_t m = (dt.getMonth() > 0 && dt.getMonth() <= 12) ? dt.getMonth() - 1 : 0;

    if (m_dateLabel) {
        lv_label_set_text_fmt(m_dateLabel, "%s %02d %s", days_str[w], dt.getDay(), months_str[m]);
    }
}
