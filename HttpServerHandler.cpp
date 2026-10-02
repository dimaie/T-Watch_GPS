#include "HttpServerHandler.h"
#include "WatchCommandController.h"

HttpServerHandler::HttpServerHandler(uint16_t port)
    : m_server(port), m_controller(nullptr), m_enabled(false), m_started(false), m_port(port) {
}

HttpServerHandler::~HttpServerHandler() {
    stop();
}

void HttpServerHandler::begin(WatchCommandController *controller) {
    m_controller = controller;
    m_enabled = false; // Disabled by default on startup
    m_started = false;
}

void HttpServerHandler::start() {
    m_enabled = true;
    if (WiFi.status() == WL_CONNECTED && !m_started) {
        m_server.on("/", HTTP_GET, [this]() { handleRoot(); });
        m_server.on("/cmd", HTTP_GET, [this]() { handleCmd(); });
        m_server.on("/cmd", HTTP_POST, [this]() { handleCmd(); });
        m_server.onNotFound([this]() { handleNotFound(); });

        m_server.begin();
        m_started = true;
        Serial.printf("[HTTP] WebServer started at http://%s/\r\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println(F("[HTTP] WebServer enabled. It will activate once Wi-Fi connects."));
    }
}

void HttpServerHandler::stop() {
    m_enabled = false;
    if (m_started) {
        m_server.stop();
        m_started = false;
        Serial.println(F("[HTTP] WebServer stopped."));
    }
}

bool HttpServerHandler::isEnabled() const {
    return m_enabled;
}

void HttpServerHandler::process() {
    // Only process server requests when enabled AND Wi-Fi is connected
    if (m_enabled && WiFi.status() == WL_CONNECTED) {
        if (!m_started) {
            m_server.on("/", HTTP_GET, [this]() { handleRoot(); });
            m_server.on("/cmd", HTTP_GET, [this]() { handleCmd(); });
            m_server.on("/cmd", HTTP_POST, [this]() { handleCmd(); });
            m_server.onNotFound([this]() { handleNotFound(); });

            m_server.begin();
            m_started = true;
            Serial.printf("[HTTP] WebServer active at http://%s/\r\n", WiFi.localIP().toString().c_str());
        }
        m_server.handleClient();
    } else {
        if (m_started) {
            m_server.stop();
            m_started = false;
        }
    }
}

void HttpServerHandler::handleRoot() {
    String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>T-Watch Ultra Dashboard</title>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;background:#121212;color:#00FF66;margin:0;padding:20px;}";
    html += "h1{color:#00FF66;border-bottom:2px solid #00FF66;padding-bottom:10px;}";
    html += ".card{background:#1E1E1E;border:1px solid #00FF66;border-radius:8px;padding:15px;margin-bottom:20px;}";
    html += "input,button{font-size:16px;padding:10px;margin:5px 0;border-radius:4px;border:none;}";
    html += "input[type=text]{width: calc(100% - 24px);background:#2A2A2A;color:#FFF;border:1px solid #444;}";
    html += "button{background:#00FF66;color:#000;font-weight:bold;cursor:pointer;}";
    html += "button:hover{background:#00CC55;}";
    html += "pre{background:#000;color:#00FF66;padding:12px;border-radius:4px;overflow-x:auto;white-space:pre-wrap;}";
    html += "</style></head><body>";
    html += "<h1>T-Watch Ultra Command Center</h1>";

    html += "<div class='card'><h2>Execute Command</h2>";
    html += "<form action='/cmd' method='GET'>";
    html += "<input type='text' name='cmd' placeholder='e.g., scan wifi, sync ntp, enable gps' required>";
    html += "<br><button type='submit'>Send Command</button>";
    html += "</form></div>";

    html += "<div class='card'><h2>Quick Actions</h2>";
    html += "<a href='/cmd?cmd=scan+wifi'><button>Scan Wi-Fi</button></a> ";
    html += "<a href='/cmd?cmd=list+wifi'><button>List Saved Wi-Fi</button></a> ";
    html += "<a href='/cmd?cmd=sync+ntp'><button>Sync NTP</button></a> ";
    html += "<a href='/cmd?cmd=enable+gps'><button>Enable GPS</button></a> ";
    html += "<a href='/cmd?cmd=disable+gps'><button>Disable GPS</button></a> ";
    html += "<a href='/cmd?cmd=sync+gps'><button>Sync GPS</button></a> ";
    html += "<a href='/cmd?cmd=help'><button>Help</button></a>";
    html += "</div>";

    html += "</body></html>";
    m_server.send(200, "text/html", html);
}

void HttpServerHandler::handleCmd() {
    String cmd = "";
    if (m_server.hasArg("cmd")) {
        cmd = m_server.arg("cmd");
    } else if (m_server.hasArg("plain")) {
        cmd = m_server.arg("plain");
    }

    cmd.trim();
    String response = "";

    if (cmd.length() > 0 && m_controller) {
        response = m_controller->executeCommand(cmd);
    } else {
        response = "[HTTP] Error: No command provided.";
    }

    if (m_server.header("Accept").indexOf("text/html") != -1 || !m_server.hasArg("plain")) {
        String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<title>Command Result</title><style>body{font-family:Arial,sans-serif;background:#121212;color:#00FF66;padding:20px;}";
        html += "pre{background:#000;color:#00FF66;padding:15px;border-radius:6px;font-size:15px;white-space:pre-wrap;}";
        html += "a{color:#00FF66;font-size:18px;text-decoration:none;}</style></head><body>";
        html += "<h2>Command Result:</h2><pre>" + response + "</pre>";
        html += "<p><a href='/'>&larr; Back to Dashboard</a></p></body></html>";
        m_server.send(200, "text/html", html);
    } else {
        m_server.send(200, "text/plain", response);
    }
}

void HttpServerHandler::handleNotFound() {
    m_server.send(404, "text/plain", "404 Not Found");
}
