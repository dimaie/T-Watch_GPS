#ifndef GPS_VIEW_H
#define GPS_VIEW_H

#include "IWatchView.h"
#include <LilyGoLib.h>
#include <LV_Helper.h>

/**
 * @class GpsView
 * @brief Manages the GPS UI container, displaying status, satellite count, and location.
 */
class GpsView : public IWatchView {
public:
    GpsView();
    virtual ~GpsView();

    void create(lv_obj_t *parent = nullptr) override;
    void update() override;
    lv_obj_t *getContainer() const override { return m_container; }
    const char *getModeName() const override { return "GPS"; }

    void setEnabled(bool enabled) override { setGpsState(enabled); }
    bool isEnabled() const override { return isGpsEnabled(); }

    void setGpsState(bool enabled);
    bool isGpsEnabled() const { return m_gpsEnabled; }

private:
    lv_obj_t *m_container = nullptr;
    lv_obj_t *m_titleLabel = nullptr;
    lv_obj_t *m_statusLabel = nullptr;
    lv_obj_t *m_satsLabel = nullptr;
    lv_obj_t *m_locLabel = nullptr;

    bool m_gpsEnabled = false;
};

#endif // GPS_VIEW_H
