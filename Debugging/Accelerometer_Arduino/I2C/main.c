#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* JTAG UART */
#define JTAG_UART_BASE          0xFF201000
#define JTAG_UART_DATA_REG      0x00
#define JTAG_UART_DATA_MASK     0xFF
#define JTAG_UART_VALID_BIT     (1 << 15)

/* LEDs */
#define LEDR_BASE               0xFF200000

volatile uint32_t *jtag_uart_ptr = (uint32_t *)JTAG_UART_BASE;
volatile uint32_t *led_ptr       = (uint32_t *)LEDR_BASE;

/* Read one byte from UART */
int uart_read_byte(void) {
    uint32_t data = jtag_uart_ptr[JTAG_UART_DATA_REG / 4];
    if (data & JTAG_UART_VALID_BIT)
        return data & JTAG_UART_DATA_MASK;
    return -1;
}

int main(void) {
    char buffer[32];
    int idx = 0;

    float values[4];
    int val_index = 0;

    while (1) {
        int byte = uart_read_byte();

        if (byte >= 0) {
            char ch = (char)byte;

            /* End of one number */
            if (ch == ' ' || ch == '\n') {
                buffer[idx] = '\0';

                if (idx > 0 && val_index < 4) {
                    values[val_index++] = atof(buffer);
                }

                idx = 0;

                /* Once we have 4 values → update LEDs */
                if (val_index == 4) {
                    uint32_t leds = 0;

                    if (fabs(values[0]) > 0.7) leds |= (1 << 0);  // LED0
                    if (fabs(values[1]) > 0.7) leds |= (1 << 1);  // LED1
                    if (fabs(values[2]) > 0.7) leds |= (1 << 2);  // LED2
                    if (fabs(values[3]) > 0.7) leds |= (1 << 3);  // LED3

                    *led_ptr = leds;

                    val_index = 0;  // reset for next set
                }

            } else {
                /* Build number string */
                if (idx < sizeof(buffer) - 1) {
                    buffer[idx++] = ch;
                }
            }
        }
    }

    return 0;
}