#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

#define VGA_ADDRESS 0xB8000
#define VGA_COLS 80
#define VGA_ROWS 25

void vga_init(void);

void clear_screen(void);
void reset_cursor(void);
void move_cursor(uint8_t x, uint8_t y);

void putchar(char c);
void putchar_at(char c, uint8_t x, uint8_t y, uint8_t color);
void print(const char *s);
void print_color(const char *s, uint8_t color);
void print_at(const char *s, uint8_t x, uint8_t y, uint8_t color);

void set_color(uint8_t color);

void scroll_up(void);

#endif
