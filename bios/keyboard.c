#include "bda.h"
#include "bios.h"
#include "io.h"
#include "serial.h"
#include "utils.h"

struct keydef {
    unsigned short normal;
    unsigned short shifted;
    unsigned short ctrl;
    unsigned short alt;
};

static const struct keydef keycode_map[] = {
        [0x1c] = {0x1E61, 0x1E41, 0x1E01, 0x1E00},
        [0x32] = {0x3062, 0x3042, 0x3002, 0x3000},
        [0x21] = {0x2E63, 0x2E43, 0x2E03, 0x2E00},
        [0x23] = {0x2064, 0x2044, 0x2004, 0x2000},
        [0x24] = {0x1265, 0x1245, 0x1205, 0x1200},
        [0x2b] = {0x2166, 0x2146, 0x2106, 0x2100},
        [0x34] = {0x2267, 0x2247, 0x2207, 0x2200},
        [0x33] = {0x2368, 0x2348, 0x2308, 0x2300},
        [0x43] = {0x1769, 0x1749, 0x1709, 0x1700},
        [0x3b] = {0x246A, 0x244A, 0x240A, 0x2400},
        [0x42] = {0x256B, 0x254B, 0x250B, 0x2500},
        [0x4b] = {0x266C, 0x264C, 0x260C, 0x2600},
        [0x3a] = {0x326D, 0x324D, 0x320D, 0x3200},
        [0x31] = {0x316E, 0x314E, 0x310E, 0x3100},
        [0x44] = {0x186F, 0x184F, 0x180F, 0x1800},
        [0x4d] = {0x1970, 0x1950, 0x1910, 0x1900},
        [0x15] = {0x1071, 0x1051, 0x1011, 0x1000},
        [0x2d] = {0x1372, 0x1352, 0x1312, 0x1300},
        [0x1b] = {0x1F73, 0x1F53, 0x1F13, 0x1F00},
        [0x2c] = {0x1474, 0x1454, 0x1414, 0x1400},
        [0x3c] = {0x1675, 0x1655, 0x1615, 0x1600},
        [0x2a] = {0x2F76, 0x2F56, 0x2F16, 0x2F00},
        [0x1d] = {0x1177, 0x1157, 0x1117, 0x1100},
        [0x22] = {0x2D78, 0x2D58, 0x2D18, 0x2D00},
        [0x35] = {0x1579, 0x1559, 0x1519, 0x1500},
        [0x1a] = {0x2C7A, 0x2C5A, 0x2C1A, 0x2C00},
        [0x16] = {0x0231, 0x0221, 0x0231, 0x7800},
        [0x1e] = {0x0332, 0x0340, 0x0300, 0x7900},
        [0x26] = {0x0433, 0x0423, 0x0433, 0x7A00},
        [0x25] = {0x0534, 0x0524, 0x0534, 0x7B00},
        [0x2e] = {0x0635, 0x0625, 0x0635, 0x7C00},
        [0x36] = {0x0736, 0x075E, 0x071E, 0x7D00},
        [0x3d] = {0x0837, 0x0826, 0x0837, 0x7E00},
        [0x3e] = {0x0938, 0x092A, 0x0938, 0x7F00},
        [0x46] = {0x0A39, 0x0A28, 0x0a39, 0x8000},
        [0x45] = {0x0B30, 0x0B29, 0x0b30, 0x8100},
        [0x4e] = {0x0C2D, 0x0C5F, 0x0C1F, 0x8200},
        [0x55] = {0x0D3D, 0x0D2B, 0x0d2b, 0x8300},
        [0x54] = {0x1A5B, 0x1A7B, 0x1A1B, 0x1A00},
        [0x5b] = {0x1B5D, 0x1B7D, 0x1B1D, 0x1B00},
        [0x4c] = {0x273B, 0x273A, 0x273b, 0x2700},
        [0x52] = {0x2827, 0x2822, 0x2827, 0x2827},
        [0x0e] = {0x2960, 0x297E, 0x2960, 0x2960},
        [0x5d] = {0x2B5C, 0x2B7C, 0x2B1C, 0x2600},
        [0x41] = {0x332C, 0x333C, 0x332c, 0x332c},
        [0x49] = {0x342E, 0x343E, 0x342e, 0x342e},
        [0x4a] = {0x352F, 0x353F, 0x352f, 0x352f},
        [0x05] = {0x3B00, 0x5400, 0x5E00, 0x6800},
        [0x06] = {0x3C00, 0x5500, 0x5F00, 0x6900},
        [0x04] = {0x3D00, 0x5600, 0x6000, 0x6A00},
        [0x0c] = {0x3E00, 0x5700, 0x6100, 0x6B00},
        [0x03] = {0x3F00, 0x5800, 0x6200, 0x6C00},
        [0x0b] = {0x4000, 0x5900, 0x6300, 0x6D00},
        [0x83] = {0x4100, 0x5A00, 0x6400, 0x6E00},
        [0x0a] = {0x4200, 0x5B00, 0x6500, 0x6F00},
        [0x01] = {0x4300, 0x5C00, 0x6600, 0x7000},
        [0x09] = {0x4400, 0x5D00, 0x6700, 0x7100},
        [0x78] = {0x8500, 0x8700, 0x8900, 0x8B00},
        [0x07] = {0x8600, 0x8800, 0x8A00, 0x8C00},
        [0x66] = {0x0E08, 0x0E08, 0x0E7F, 0x0E00},
        [0x5a] = {0x1C0D, 0x1C0D, 0x1C0A, 0xA600},
        [0x76] = {0x011B, 0x011B, 0x011B, 0x0100},
        [0x29] = {0x3920, 0x3920, 0x3920, 0x3920},
        [0x0d] = {0x0F09, 0x0F00, 0x9400, 0xA500},
};

static const struct keydef extended_keycode_map[] = {
        [0x71] = {0x5300, 0x532E, 0x9300, 0xA300},
        [0x72] = {0x5000, 0x5032, 0x9100, 0xA000},
        [0x69] = {0x4F00, 0x4F31, 0x7500, 0x9F00},
        [0x6c] = {0x4700, 0x4737, 0x7700, 0x9700},
        [0x70] = {0x5200, 0x5230, 0x9200, 0xA200},
        [0x6b] = {0x4B00, 0x4B34, 0x7300, 0x9B00},
        [0x7a] = {0x5100, 0x5133, 0x7600, 0xA100},
        [0x7d] = {0x4900, 0x4939, 0x8400, 0x9900},
        [0x74] = {0x4D00, 0x4D36, 0x7400, 0x9D00},
        [0x75] = {0x4800, 0x4838, 0x8D00, 0x9800},
};

static int kbd_buffer_full(void)
{
    unsigned short tail = bda_read(kbd_buffer_tail);
    unsigned short head = bda_read(kbd_buffer_head);

    // clang-format off
    return (tail == head - 2) ||
           (head == offsetof(struct bios_data_area, kbd_buffer) &&
            tail == (offsetof(struct bios_data_area, drive_recalibration_status) - 2));
    // clang-format on
}

void __attribute__((noinline)) kbd_buffer_add(unsigned short key)
{
    if (kbd_buffer_full())
        return;

    unsigned short tail = bda_read(kbd_buffer_tail);
    unsigned circ_offset = tail - offsetof(struct bios_data_area, kbd_buffer);
    bda_write(kbd_buffer[circ_offset / sizeof(unsigned short)], key);

    tail += sizeof(unsigned short);
    if (tail >= offsetof(struct bios_data_area, drive_recalibration_status))
        tail = offsetof(struct bios_data_area, kbd_buffer);
    bda_write(kbd_buffer_tail, tail);
}

static void kbd_buffer_pop(void)
{
    unsigned short head = bda_read(kbd_buffer_head);

    head += sizeof(unsigned short);
    if (head >= offsetof(struct bios_data_area, drive_recalibration_status))
        head = offsetof(struct bios_data_area, kbd_buffer);
    bda_write(kbd_buffer_head, head);
}

unsigned kbd_buffer_peek(void)
{
    unsigned short head = bda_read(kbd_buffer_head);
    unsigned short tail = bda_read(kbd_buffer_tail);

    if (head == tail)
        return 0;

    unsigned circ_offset = head - offsetof(struct bios_data_area, kbd_buffer);

    return bda_read(kbd_buffer[circ_offset / sizeof(unsigned short)]);
}

#define SCANCODE_LSHIFT 0x12
#define SCANCODE_LCTRL 0x14
#define SCANCODE_LALT 0x11

#define KBD_FLAG_LSHIFT (1 << 1)
#define KBD_FLAG_LCTRL (1 << 2)
#define KBD_FLAG_LALT (1 << 3)

static int is_modifier(unsigned char b)
{
    return b == SCANCODE_LALT || b == SCANCODE_LCTRL || b == SCANCODE_LSHIFT;
}

void modifier_key(unsigned char b)
{
    unsigned char flags = bda_read(keyboard_flags[0]);

    if (b == SCANCODE_LALT)
        flags ^= KBD_FLAG_LALT;
    if (b == SCANCODE_LSHIFT)
        flags ^= KBD_FLAG_LSHIFT;
    if (b == SCANCODE_LCTRL)
        flags ^= KBD_FLAG_LCTRL;

    bda_write(keyboard_flags[0], flags);
}

static int __attribute__((noinline)) get_kbd_flags(void)
{
    return bda_read(keyboard_flags[0]);
}

static void __attribute__((noinline))
keypress(const struct keydef *map, unsigned char b)
{
    const struct keydef *def = &map[b];
    unsigned short flags = get_kbd_flags();

    if ((flags & KBD_FLAG_LSHIFT) && def->shifted)
        kbd_buffer_add(def->shifted);
    else if ((flags & KBD_FLAG_LCTRL) && def->ctrl)
        kbd_buffer_add(def->ctrl);
    else if ((flags & KBD_FLAG_LALT) && def->alt)
        kbd_buffer_add(def->alt);
    else if (def->normal)
        kbd_buffer_add(def->normal);
}

static unsigned char get_scancode(void)
{
    unsigned char b;

    do {
        b = inb(0x60);
        if (b)
            outb(0x61, 0);
    } while (!b);

    return b;
}

static int is_extended(unsigned char b)
{
    return b == 0xe0;
}

static void extended_key(void)
{
    keypress(extended_keycode_map, get_scancode());
}

static void keyboard_poll(void)
{
    unsigned char b = inb(0x60);

    if (!b)
        return;

    outb(0x61, 0);

    int keyup = 0;
    if (b == 0xf0) {
        keyup = 1;
        b = get_scancode();
    }

    if (is_modifier(b))
        modifier_key(b);
    else if (is_extended(b))
        extended_key();
    else if (!keyup && b < ARRAY_SIZE(keycode_map))
        keypress(keycode_map, b);
}

static void kbd_irq(struct callregs *regs)
{
    keyboard_poll();
}
VECTOR(0x9, kbd_irq);

static void keyboard_wait(struct callregs *regs)
{
    unsigned short c;

    regs->flags &= ~CF;
    do {
        c = kbd_buffer_peek();
        if (!c) {
            serial_poll();
            keyboard_poll();
        }
    } while (c == 0);

    kbd_buffer_pop();

    regs->ax.x = c;
}

static void keyboard_status(struct callregs *regs)
{
    unsigned short c;

    regs->flags &= ~CF;

    c = kbd_buffer_peek();
    if (!c)
        serial_poll();
    c = kbd_buffer_peek();
    if (c)
        regs->flags &= ~ZF;
    else
        regs->flags |= ZF;

    regs->ax.x = c;
}

static void keyboard_shift_status(struct callregs *regs)
{
    regs->flags &= ~CF;
    regs->ax.h = 0;
    regs->ax.l = bda_read(keyboard_flags[0]);
}

static void keyboard_services(struct callregs *regs)
{
    regs->flags |= CF;

    switch (regs->ax.h) {
    case 0x0:  // Fallthrough
    case 0x10: keyboard_wait(regs); break;
    case 0x1:  // Fallthrough
    case 0x11: keyboard_status(regs); break;
    case 0x2: keyboard_shift_status(regs); break;
    default: break;
    }
}
VECTOR(0x16, keyboard_services);

void keyboard_init(void)
{
    bda_write(keyboard_flags[0], 0);
    bda_write(keyboard_flags[1], 0);
    bda_write(kbd_buffer_tail, offsetof(struct bios_data_area, kbd_buffer));
    bda_write(kbd_buffer_head, offsetof(struct bios_data_area, kbd_buffer));

    // Enable IRQ
    outb(0xfff4, inb(0xfff4) | 2);
}
