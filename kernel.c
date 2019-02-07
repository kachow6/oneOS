#include <stddef.h>
#include <stdint.h>

/*  MMIO - Memory Mapped IO
    interacting with hardware devices via read/writes to predefined mem. addresses */

static inline void mmio_write(uint32_t reg, uint32_t data)
{
    *(volatile uint32_t*)reg = data;
}

static inline uint32_t mmio_read(uint32_t reg)
{
    return *(volatile uint32_t*)reg;
}

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
            : "=r"(count): [count]"0"(count) : "cc");
}

/*  Sets up hardware for basic I/O
    UART hardware allows text data communication through serial ports */
enum
{
    // The GPIO registers peripheral base address.
    GPIO_BASE = 0x3F200000, // for raspi2 & 3, 0x20200000 for raspi1

    GPPUD = (GPIO_BASE + 0x94),
    GPPUDCLK0 = (GPIO_BASE + 0x98),

    // The base address for UART.
    UART0_BASE = 0x3F201000, // for raspi2 & 3, 0x20201000 for raspi1

    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C),
};

/* Sets up UART hardware for use */
void uart_init()
{
    /* Disable all UART hardware */
    mmio_write(UART0_CR, 0x00000000);

    mmio_write(GPPUD, 0x00000000);
    delay(150);

    /* Disable (GPPUD) pins 14 and 15 of GPIO */
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    mmio_write(GPPUDCLK0, 0x00000000);

    /* Set all flags in Interrupt Clear Register (clear all pending interrupts) */
    mmio_write(UART0_ICR, 0x7FF);

    /* Set baud rate of connection (bps across serial port) */
    mmio_write(UART0_IBRD, 1);
    mmio_write(UART0_FBRD, 40);

    /*  Writes bits 4-6 to Line control register
        4       => hold data in 8 item deep FIFO (instead of 1 item)
        5 & 6   => sent/received data = 8-bit long words */
    mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    /* Disable all interrupts from UART by writing 1 to bits of Interrupt Mask Set Clear register */
    mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
            (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    /*  Write bits 0, 8, 9 to control register
        0 => enable UART hardware
        8 => enable receive data
        9 => enable transmit data */
    mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

/* Check flag register, then write char to data register */
void uart_putc(unsigned char c)
{
    while ( mmio_read(UART0_FR) & (1 << 5) ) { }
    mmio_write(UART0_DR, c);
}

/* Check flag register, then read char from data register */
unsigned char uart_getc()
{
    while ( mmio_read(UART0_FR) & (1 << 4) ) { }
    return mmio_read(UART0_DR);
}

/* Write string to data register */
void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putc((unsigned char)str[i]);
}

/* Main function (control from boot.S) */
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void) r0;
    (void) r1;
    (void) atags; /* Info about hardware and command line used to run kernel in memory */

    uart_init();
    uart_puts("Hello, welcome to one OS\r\n");

    while (1) {
        uart_putc(uart_getc());
        uart_putc('\n');
    }
}