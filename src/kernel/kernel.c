#include <stdint.h>
#include <kernel/uart.h>

/* Main function (control from boot.S) */
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void) r0;
    (void) r1;
    (void) atags; /* Info about hardware and command line used to run kernel in memory */

    uart_init();
    uart_puts("Hello, welcome to oneOS\r\n");

    while (1) {
        uart_putc(uart_getc());
    }
}