#include "UartCommandHandler.h"

UartCommandHandler::UartCommandHandler() {
}

UartCommandHandler::~UartCommandHandler() {
}

void UartCommandHandler::begin(WatchCommandController *controller, Stream &serial) {
    m_controller = controller;
    m_serial = &serial;
    m_rxBuffer.reserve(128);
    m_serial->print(F("\r\n==============================================\r\n"));
    m_serial->print(F("  T-Watch Ultra UART & HTTP Command System Ready\r\n"));
    m_serial->print(F("  Type 'help' for available commands.\r\n"));
    m_serial->print(F("==============================================\r\n"));
    m_serial->print(F("> "));
}

void UartCommandHandler::process() {
    while (m_serial && m_serial->available()) {
        char c = (char)m_serial->read();

        // Handle Carriage Return as line execution
        if (c == '\r') {
            m_serial->print(F("\r\n")); // Echo CR+LF
            m_rxBuffer.trim();
            if (m_rxBuffer.length() > 0 && m_controller) {
                String out = m_controller->executeCommand(m_rxBuffer);
                m_serial->print(out);
                m_rxBuffer = "";
            }
            m_serial->print(F("> ")); // Prompt for next input
            continue;
        }

        // Ignore Line Feed if it follows Carriage Return
        if (c == '\n') {
            continue;
        }

        // Handle Backspace / Delete
        if (c == '\b' || c == (char)0x7F) {
            if (m_rxBuffer.length() > 0) {
                m_rxBuffer.remove(m_rxBuffer.length() - 1);
                m_serial->print(F("\b \b")); // Visually erase character in terminal
            }
        }
        // Echo printable characters
        else if (c >= 32 && c <= 126) {
            m_rxBuffer += c;
            m_serial->write(c); // Local echo back to Tera Term
        }
    }
}
