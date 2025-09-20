// fb.c - framebuffer implementation
#include "fb.h"
#include <stdint.h>
#include <stddef.h>
#include "string.h" // for memcpy if you have it; otherwise the loop will be used

static volatile uint8_t *fb_ptr = (volatile uint8_t *)0;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static uint32_t fb_pitch = 0;
static uint8_t fb_bpp = 0;

// Backbuffer pointer (uint32_t assumed for 32bpp)
static uint32_t *backbuffer = 0;

void fb_init(uintptr_t phys_addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp)
{
    fb_ptr = (volatile uint8_t *)phys_addr;
    fb_width = width;
    fb_height = height;
    fb_pitch = pitch;
    fb_bpp = bpp;
    // Note: we don't allocate a backbuffer here (kernel should provide memory if desired).
}

void fb_set_backbuffer(void *backbuffer_ptr)
{
    backbuffer = (uint32_t *)backbuffer_ptr;
}

// Minimal helper to write a pixel into backbuffer (or directly to fb if backbuffer == NULL).
void fb_putpixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= (int)fb_width || y < 0 || y >= (int)fb_height)
        return;

    if (backbuffer)
    {
        backbuffer[y * fb_width + x] = color;
        return;
    }

    // No backbuffer: write directly to framebuffer memory. Handle 32bpp and 24bpp/16bpp minimally.
    if (fb_bpp == 32)
    {
        volatile uint32_t *dst = (volatile uint32_t *)(fb_ptr + y * fb_pitch) + x;
        *dst = color;
    }
    else if (fb_bpp == 24)
    {
        uint8_t *dst = (uint8_t *)(fb_ptr + y * fb_pitch) + x * 3;
        dst[0] = color & 0xFF;
        dst[1] = (color >> 8) & 0xFF;
        dst[2] = (color >> 16) & 0xFF;
    }
    else if (fb_bpp == 16)
    {
        // assume 5:6:5
        uint16_t r = (color >> 19) & 0x1F;
        uint16_t g = (color >> 10) & 0x3F;
        uint16_t b = (color >> 3) & 0x1F;
        uint16_t pix = (r << 11) | (g << 5) | b;
        volatile uint16_t *dst = (volatile uint16_t *)(fb_ptr + y * fb_pitch) + x;
        *dst = pix;
    }
    else
    {
        // unsupported bpp: do nothing
    }
}

void fb_hline(int x, int y, int length, uint32_t color)
{
    if (backbuffer)
    {
        for (int i = 0; i < length; ++i)
            backbuffer[y * fb_width + (x + i)] = color;
    }
    else
    {
        for (int i = 0; i < length; ++i)
            fb_putpixel(x + i, y, color);
    }
}

void fb_fill_rect(int x, int y, int w, int h, uint32_t color)
{
    for (int row = 0; row < h; ++row)
    {
        fb_hline(x, y + row, w, color);
    }
}

void fb_swap_buffers(void)
{
    if (!backbuffer || !fb_ptr)
        return;

    // If pitch == width * 4 and 32bpp we can memcpy the whole block
    if (fb_bpp == 32 && fb_pitch == fb_width * 4)
    {
        // Use your memcpy in string.c if available
        memcpy((void *)fb_ptr, backbuffer, (size_t)fb_width * fb_height * 4);
        return;
    }

    // Otherwise copy row by row considering pitch
    for (uint32_t row = 0; row < fb_height; ++row)
    {
        uint8_t *dst = (uint8_t *)fb_ptr + row * fb_pitch;
        uint32_t *src = backbuffer + row * fb_width;
        if (fb_bpp == 32)
        {
            // copy width*4 bytes
            for (uint32_t col = 0; col < fb_width; ++col)
            {
                ((volatile uint32_t *)dst)[col] = src[col];
            }
        }
        else if (fb_bpp == 24)
        {
            for (uint32_t col = 0; col < fb_width; ++col)
            {
                uint32_t c = src[col];
                dst[col * 3 + 0] = c & 0xFF;
                dst[col * 3 + 1] = (c >> 8) & 0xFF;
                dst[col * 3 + 2] = (c >> 16) & 0xFF;
            }
        }
        else if (fb_bpp == 16)
        {
            for (uint32_t col = 0; col < fb_width; ++col)
            {
                uint32_t c = src[col];
                uint16_t r = (c >> 19) & 0x1F;
                uint16_t g = (c >> 10) & 0x3F;
                uint16_t b = (c >> 3) & 0x1F;
                uint16_t pix = (r << 11) | (g << 5) | b;
                ((volatile uint16_t *)dst)[col] = pix;
            }
        }
    }
}
