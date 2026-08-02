#include "stdafx.h"

#include "ps1/registers.h"

#define SIO0_ADDR_CONTROLLER 0x01
#define SIO0_PAD_POLL 'B'
#define DTR_DELAY_US 60
#define DSR_TIMEOUT_US 120

typedef struct input_manager_t {
    unsigned int new_keystate, old_keystate;
    unsigned int keys_pressed, keys_held, keys_released;
} InputManager;

static InputManager input_manager;

static void delayMicroseconds(int time)
{
    time = ((time * 271) + 4) / 8;

    __asm__ volatile(
        ".set push\n"
        ".set noreorder\n"
        "bgtz  %0, .\n"
        "addiu %0, -2\n"
        ".set pop\n"
        : "+r"(time));
}

static bool waitForAcknowledge(int timeout)
{
    for (; timeout > 0; timeout -= 10) {
        if (IRQ_STAT & (1 << IRQ_SIO0)) {
            IRQ_STAT = ~(1 << IRQ_SIO0);
            SIO_CTRL(0) |= SIO_CTRL_ACKNOWLEDGE;
            return true;
        }

        delayMicroseconds(10);
    }

    return false;
}

static uint8_t exchangeByte(uint8_t value)
{
    while (!(SIO_STAT(0) & SIO_STAT_TX_NOT_FULL)) __asm__ volatile("");
    SIO_DATA(0) = value;
    while (!(SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY)) __asm__ volatile("");
    return SIO_DATA(0);
}

static size_t exchangeControllerPacket(const uint8_t* request, uint8_t* response, size_t request_length, size_t response_capacity)
{
    IRQ_STAT = ~(1 << IRQ_SIO0);
    SIO_CTRL(0) &= ~SIO_CTRL_CS_PORT_2;
    SIO_CTRL(0) |= SIO_CTRL_DTR | SIO_CTRL_ACKNOWLEDGE;
    delayMicroseconds(DTR_DELAY_US);

    size_t response_length = 0;
    SIO_DATA(0) = SIO0_ADDR_CONTROLLER;

    if (waitForAcknowledge(DSR_TIMEOUT_US)) {
        while (SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY) SIO_DATA(0);

        while (response_length < response_capacity) {
            uint8_t value = request_length ? *request++ : 0;
            if (request_length) request_length--;

            *response++ = exchangeByte(value);
            response_length++;

            if (!waitForAcknowledge(DSR_TIMEOUT_US)) break;
        }
    }

    delayMicroseconds(DTR_DELAY_US);
    SIO_CTRL(0) &= ~SIO_CTRL_DTR;
    return response_length;
}

static void updateButtonState(unsigned int state)
{
    input_manager.old_keystate = input_manager.new_keystate;
    input_manager.new_keystate = state;
    input_manager.keys_pressed = (input_manager.old_keystate ^ state) & state;
    input_manager.keys_held = state;
    input_manager.keys_released = (input_manager.old_keystate ^ state) & input_manager.old_keystate;
}

void pad_init(void)
{
    SIO_CTRL(0) = SIO_CTRL_RESET;
    SIO_MODE(0) = SIO_MODE_BAUD_DIV1 | SIO_MODE_DATA_8;
    SIO_BAUD(0) = F_CPU / 250000;
    SIO_CTRL(0) = SIO_CTRL_TX_ENABLE | SIO_CTRL_RX_ENABLE | SIO_CTRL_DSR_IRQ_ENABLE;

    memset(&input_manager, 0, sizeof(input_manager));
}

void pad_pollEvents(void)
{
    static const uint8_t request[] = {SIO0_PAD_POLL, 0, 0, 0};
    uint8_t response[8] = {0};
    size_t response_length = exchangeControllerPacket(request, response, sizeof(request), sizeof(response));

    if (response_length >= 4 && response[1] == 0x5a) {
        unsigned int buttons = ((unsigned int)response[2] | ((unsigned int)response[3] << 8)) ^ 0xffffu;
        updateButtonState(buttons);
    } else {
        updateButtonState(0);
    }
}

int pad_isHeld(int key)
{
    return (1u << key) & input_manager.keys_held;
}

int pad_isPressed(int key)
{
    return (1u << key) & input_manager.keys_pressed;
}

int pad_isReleased(int key)
{
    return (1u << key) & input_manager.keys_released;
}
