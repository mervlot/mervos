#include "vga.h"
#include <stdint.h>
#include <stddef.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static volatile uint16_t *vga_buffer = (volatile uint16_t*)VGA_ADDRESS;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t default_attr = 0x07;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

static void set_hardware_cursor_shape(uint8_t start, uint8_t end) {

    outb(0x3D4, 0x0A);
    outb(0x3D5, start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, end);
}

static void update_hardware_cursor(void) {
    uint16_t pos = (uint16_t)(cursor_y * VGA_COLS + cursor_x);
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}


static void scroll_if_needed(void) {
    if (cursor_y < VGA_ROWS) return;

    for (size_t y = 1; y < VGA_ROWS; ++y) {
        for (size_t x = 0; x < VGA_COLS; ++x) {
            vga_buffer[(y - 1) * VGA_COLS + x] = vga_buffer[y * VGA_COLS + x];
        }
    }

    for (size_t x = 0; x < VGA_COLS; ++x) {
        vga_buffer[(VGA_ROWS - 1) * VGA_COLS + x] = vga_entry(' ', default_attr);
    }

    cursor_y = VGA_ROWS - 1;
    update_hardware_cursor();
}



void vga_init(void) {

    clear_screen();
    set_hardware_cursor_shape(0x0E, 0x0F); 
    update_hardware_cursor();
}

void clear_screen(void) {
    for (size_t i = 0; i < VGA_COLS * VGA_ROWS; ++i) {
        vga_buffer[i] = vga_entry(' ', default_attr);
    }
    cursor_x = 0;
    cursor_y = 0;
    update_hardware_cursor();
}

void move_cursor(uint8_t x, uint8_t y) {
    if (x >= VGA_COLS || y >= VGA_ROWS) return;
    cursor_x = x;
    cursor_y = y;
    update_hardware_cursor();
}

void reset_cursor(void) {
    cursor_x = 0;
    cursor_y = 0;
    update_hardware_cursor();
}

void set_color(uint8_t color) {
    default_attr = color;
}

void putchar(char c) {
    if (c == '\n') {     
        cursor_x = 0;
        cursor_y++;
        scroll_if_needed();
        update_hardware_cursor();
        return;
    }

    if (c == '\r') {       
        cursor_x = 0;
        update_hardware_cursor();
        return;
    }

    if (c == '\b') {        
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_COLS - 1;
        }
        vga_buffer[cursor_y * VGA_COLS + cursor_x] = vga_entry(' ', default_attr);
        update_hardware_cursor();
        return;
    }

    if (c == '\t') {         
        const uint8_t tab_size = 4;
        for (uint8_t i = 0; i < tab_size; ++i) putchar(' ');
        return; 
    }

    vga_buffer[cursor_y * VGA_COLS + cursor_x] = vga_entry(c, default_attr);
    cursor_x++;
    if (cursor_x >= VGA_COLS) {
        cursor_x = 0;
        cursor_y++;
        scroll_if_needed();
    }
    update_hardware_cursor();
}

void putchar_at(char c, uint8_t x, uint8_t y, uint8_t color) {
    if (x >= VGA_COLS || y >= VGA_ROWS) return;
    vga_buffer[y * VGA_COLS + x] = vga_entry(c, color);
}

void print(const char *s) {
    while (*s) putchar(*s++);
}

void print_color(const char *s, uint8_t color) {
    uint8_t old = default_attr;
    default_attr = color;
    print(s);
    default_attr = old;
}

void print_at(const char* s, uint8_t x, uint8_t y, uint8_t color) {
    move_cursor(x, y);
    uint8_t old = default_attr;
    default_attr = color;
    print(s);
    default_attr = old;
}

void scroll_up(void) {
    for (size_t y = 1; y < VGA_ROWS; ++y) {
        for (size_t x = 0; x < VGA_COLS; ++x) {
            vga_buffer[(y - 1) * VGA_COLS + x] = vga_buffer[y * VGA_COLS + x];
        }
    }
    for (size_t x = 0; x < VGA_COLS; ++x) {
        vga_buffer[(VGA_ROWS - 1) * VGA_COLS + x] = vga_entry(' ', default_attr);
    }
    if (cursor_y > 0) cursor_y--;
    update_hardware_cursor();
}
