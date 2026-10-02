#ifndef UART_COMMAND_HANDLER_H
#define UART_COMMAND_HANDLER_H

#include <Arduino.h>
#include "WatchCommandController.h"

/**
 * @class UartCommandHandler
 * @brief Handles UART/Serial input streams and routes commands to WatchCommandController.
 */
class UartCommandHandler {
public:
    UartCommandHandler();
    ~UartCommandHandler();

    /**
     * @brief Initialize UART command stream.
     * @param controller Reference to the shared WatchCommandController instance.
     * @param serial Reference to hardware serial stream (defaults to Serial).
     */
    void begin(WatchCommandController *controller, Stream &serial = Serial);

    /**
     * @brief Polls the UART interface for incoming command lines. Call in loop().
     */
    void process();

private:
    WatchCommandController *m_controller = nullptr;
    Stream *m_serial = &Serial;
    String m_rxBuffer;
};

#endif // UART_COMMAND_HANDLER_H
