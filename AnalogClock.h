#ifndef ANALOG_CLOCK_H
#define ANALOG_CLOCK_H

#include "IWatchView.h"
#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <math.h>

/**
 * @class AnalogClock
 * @brief Manages the green-on-black analog watch face UI and RTC hand updates.
 */
class AnalogClock : public IWatchView {
public:
    AnalogClock();
    virtual ~AnalogClock();

    void create(lv_obj_t *parent = nullptr) override;
    void update() override { updateTime(); }
    void updateTime();
    void updateStatusLabel();

    lv_obj_t *getContainer() const override { return m_container; }
    const char *getModeName() const override { return "CLOCK"; }

private:
    lv_obj_t *m_container = nullptr;
    lv_obj_t *m_dialRing = nullptr;
    lv_obj_t *m_hourLine = nullptr;
    lv_obj_t *m_minLine = nullptr;
    lv_obj_t *m_secLine = nullptr;
    lv_obj_t *m_dateLabel = nullptr;
    lv_obj_t *m_statusLabel = nullptr;
    lv_obj_t *m_centerCap = nullptr;

    lv_point_precise_t m_hourPts[2];
    lv_point_precise_t m_minPts[2];
    lv_point_precise_t m_secPts[2];
    lv_point_precise_t m_minTickPts[60][2];

    int32_t m_cx = 0;
    int32_t m_cy = 0;
    int32_t m_dialR = 0;
};

#endif // ANALOG_CLOCK_H
