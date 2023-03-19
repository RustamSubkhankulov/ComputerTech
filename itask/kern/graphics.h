#ifndef JOS_KERN_GRAPHICS_H
#define JOS_KERN_GRAPHICS_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/graphics.h>
#include <kern/vbe.h>

// Support for bpp != 32 is not implemented yet !!!

void srfc_blit(srfc_t* surface, void* buffer, uint16_t bpp, pair16_t dst, pair16_t copyres);

void srfc_set_pxl(srfc_t* surface, pair16_t coords, color32bpp_t color);

void srfc_fill(srfc_t* surface, color32bpp_t color);
void srfc_clear(srfc_t* surface);
void srfc_copy(srfc_t* ssurface, srfc_t* dsurface, pair16_t src, pair16_t dst, size_t count);

void srfc_hzline(srfc_t* surface, uint16_t x1, uint16_t x2, uint16_t y, color32bpp_t color);
void srfc_vtline(srfc_t* surface, uint16_t y1, uint16_t y2, uint16_t x, color32bpp_t color);
void srfc_line(srfc_t* surface, pair16_t start, pair16_t end, color32bpp_t color);

// void srfc_line_thick(srfc_t* surface, pair16_t start, pair16_t end, color32bpp_t color, uint16_t thickness);

void srfc_box(srfc_t* surface, pair16_t bl, pair16_t tr, color32bpp_t color);
void srfc_box_thick_in(srfc_t* surface, pair16_t bl, pair16_t tr, color32bpp_t color, uint16_t thickness);

void srfc_bar(srfc_t* surface, pair16_t bl, pair16_t tr, color32bpp_t color);

void srfc_cf(srfc_t* surface, pair16_t center, uint16_t rad, color32bpp_t color);
void srfc_circle(srfc_t* surface, pair16_t center, uint16_t rad, color32bpp_t color);
void srfc_cf_thick_in(srfc_t* surface, pair16_t center, uint16_t rad, uint16_t thickness, color32bpp_t color);

#endif // JOS_KERN_GRAPHICS_H