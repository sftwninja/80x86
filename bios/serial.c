#include "io.h"

#define UART_DATA_PORT 0xfffa
#define UART_STATUS_PORT 0xfffb
#define UART_TX_BUSY 0x02
#define UART_RX_READY 0x01

void putchar(unsigned char c)
{
    outb(UART_DATA_PORT, c);

    while (inb(UART_STATUS_PORT) & UART_TX_BUSY)
        continue;
}

unsigned char getchar(void)
{
    while (!(inb(UART_STATUS_PORT) & UART_RX_READY))
        continue;

    return inb(UART_DATA_PORT);
}

int getchar_ready(void)
{
    return inb(UART_STATUS_PORT) & UART_RX_READY;
}

void putbyte(unsigned char b)
{
    static const char hexarr[] = "0123456789abcdef";

    putchar(read_csbyte(hexarr + ((b & 0xf0) >> 4)));
    putchar(read_csbyte(hexarr + (b & 0x0f)));
}

void putstr(const char *str)
{
    char p;

    for (p = read_csbyte(str); p != 0; ++str, p = read_csbyte(str))
        putchar(p);
}