#ifndef I_WATCH_VIEW_H
#define I_WATCH_VIEW_H

#include <LilyGoLib.h>
#include <LV_Helper.h>

/**
 * @class IWatchView
 * @brief Abstract interface for all watch mode view containers.
 */
class IWatchView {
public:
    virtual ~IWatchView() {}

    /**
     * @brief Creates LVGL UI elements inside its root container.
     */
    virtual void create(lv_obj_t *parent = nullptr) = 0;

    /**
     * @brief Periodic update function for redrawing/refreshing view content.
     */
    virtual void update() = 0;

    /**
     * @brief Returns the root LVGL container owned by this view.
     */
    virtual lv_obj_t *getContainer() const = 0;

    /**
     * @brief Returns human-readable mode name (e.g. "CLOCK", "COMPASS", "GPS").
     */
    virtual const char *getModeName() const = 0;

    /**
     * @brief Polymorphic enablement control for mode views.
     */
    virtual void setEnabled(bool enabled) { (void)enabled; }
    virtual bool isEnabled() const { return true; }
};

#endif // I_WATCH_VIEW_H
