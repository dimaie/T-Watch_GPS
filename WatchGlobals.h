#ifndef WATCH_GLOBALS_H
#define WATCH_GLOBALS_H

class AnalogClock;
class GpsView;
class WatchModeManager;
class WatchCommandController;
class HttpServerHandler;

// Global singletons available across the application
extern AnalogClock analogClock;
extern GpsView gpsView;
extern WatchModeManager modeManager;
extern WatchCommandController commandController;
extern HttpServerHandler httpHandler;

#endif // WATCH_GLOBALS_H
