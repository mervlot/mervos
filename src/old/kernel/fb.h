// fb.h - simple framebuffer API (pixel drawing + optional backbuffer)
#ifndef FB_H
#define FB_H

#include <stdint.h>
#include <stddef.h>

// Initialize the framebuffer abstraction.
//
// phys_addr: physical address of the linear framebuffer (provided by bootloader/GRUB)
// width/height: pixel dimensions
// pitch: bytes per scanline (useful for non-packed layouts)
// bpp: bits per pixel (e.g., 32)
void fb_init(uintptr_t phys_addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp);

// Optionally set a backbuffer pointer (RAM). If not set, drawing writes directly to framebuffer.
void fb_set_backbuffer(void *backbuffer_ptr);

// Put a pixel into the backbuffer (or directly to fb if no backbuffer)
void fb_putpixel(int x, int y, uint32_t color);

// Fill a rectangle (uses fb_putpixel)
void fb_fill_rect(int x, int y, int w, int h, uint32_t color);

// Copy backbuffer to the real framebuffer. If no backbuffer set this is a no-op.
void fb_swap_buffers(void);

// Helper: draw a fast horizontal line into backbuffer
void fb_hline(int x, int y, int length, uint32_t color);

#endif
