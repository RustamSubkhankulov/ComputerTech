#ifndef JOS_KERN_GRAPHICS_H
#define JOS_KERN_GRAPHICS_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/graphics.h>
#include <kern/vbe.h>

// Support for bpp != 32 is not implemented yet !!!

void surface_blit(surface_t* surface, void* buffer, pair16_t src, uint16_t bpp, pair16_t dst, pair16_t copyres);

void surface_set_pxl(surface_t* surface, pair16_t coords, color32bpp_t color);

void surface_fill(surface_t* surface, color32bpp_t color);
void surface_clear(surface_t* surface);
void surface_copy(surface_t* ssurface, surface_t* dsurface, pair16_t src, pair16_t dst, size_t count);

void surface_hzline(surface_t* surface, uint16_t x1, uint16_t x2, uint16_t y, color32bpp_t color);
void surface_vtline(surface_t* surface, uint16_t y1, uint16_t y2, uint16_t x, color32bpp_t color);
void surface_line(surface_t* surface, pair16_t start, pair16_t end, color32bpp_t color);

// void surface_line_thick(surface_t* surface, pair16_t start, pair16_t end, color32bpp_t color, uint16_t thickness);

void surface_box(surface_t* surface, pair16_t bl, pair16_t tr, color32bpp_t color);
void surface_box_thick_in(surface_t* surface, pair16_t bl, pair16_t tr, color32bpp_t color, uint16_t thickness);

void surface_bar(surface_t* surface, pair16_t bl, pair16_t tr, color32bpp_t color);

void surface_cf(surface_t* surface, pair16_t center, uint16_t rad, color32bpp_t color);
void surface_circle(surface_t* surface, pair16_t center, uint16_t rad, color32bpp_t color);
void surface_cf_thick_in(surface_t* surface, pair16_t center, uint16_t rad, uint16_t thickness, color32bpp_t color);

#endif // JOS_KERN_GRAPHICS_H