#ifndef HTTP_SERVER_HANDLER_H
#define HTTP_SERVER_HANDLER_H

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

class WatchCommandController;

/**
 * @class HttpServerHandler
 * @brief HTTP Web Server interface accepting watch commands via REST and Web UI.
 *        Disabled by default; start/stop controlled independently via commands.
 */
class HttpServerHandler {
public:
    HttpServerHandler(uint16_t port = 80);
    ~HttpServerHandler();

    void begin(WatchCommandController *controller);
    void process();

    /**
     * @brief Manually enables and starts the web server.
     */
    void start();

    /**
     * @brief Manually disables and stops the web server.
     */
    void stop();

    /**
     * @brief Returns whether the web server is enabled.
     */
    bool isEnabled() const;

private:
    void handleRoot();
    void handleCmd();
    void handleNotFound();

    WebServer m_server;
    WatchCommandController *m_controller = nullptr;
    bool m_enabled = false;
    bool m_started = false;
    uint16_t m_port = 80;
};

#endif // HTTP_SERVER_HANDLER_H
