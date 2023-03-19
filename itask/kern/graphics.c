#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/graphics.h>

#define ABS(x) (((x) > 0)? (x) : -(x))
#define SUBABS(x, y) ((x > y)? ((x) - (y)) : ((y) - (x)))
#define EQUAL_EPS(a, b, eps) (ABS((a) - (b)) < eps)

static uint32_t distance2(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

static inline size_t offs_by_coords_32bpp(pair16_t coords, uint16_t xres)
{   
    return (size_t) ((coords.y * xres + coords.x));
}

/*
    srfc_blit():
                                        surface
                                 _____________________   
                                |                     |    
     _ _ _ _   _ _ _ _          |  (dst)>______ _ _ __|_ _<
    |_|_|_|_..._|_|_|_| =>  =>  | =>  =>|      |      |    copyres.y
        void* buffer            |       |______| _ _ _|_ _<
                                |       |      |      |
                                |_______|______|______|
                                        |      |
                                        ^      ^
                                        copyres.x

*/

void srfc_blit(srfc_t* surface, void* buffer, uint16_t bpp, pair16_t dst, pair16_t copyres)
{
    assert(surface->bpp == 32);

    if (bpp != 32)
        panic("srfc_blit: support fro bpp != 32 has not been implemented yet. \n");

    assert(copyres.x < surface->res.x - dst.x);
    assert(copyres.y < surface->res.y - dst.y);

    pair16_t cur = dst;

    for (size_t iter = 0; iter < copyres.x * copyres.y; iter++)
    {
        surface->buffer[offs_by_coords_32bpp(cur, surface->res.x)] = *((uint32_t*) buffer);
    
        cur.x += 1;

        if (cur.x - dst.x >= copyres.x)
        {
            cur.x = dst.x;
            cur.y += 1;
        }
    }

    return;
}

void srfc_set_pxl(srfc_t* surface, pair16_t coords, color32bpp_t color)
{
    assert(surface->res.x > coords.x && surface->res.y > coords.y);
    assert(surface->bpp == 32);

    surface->buffer[offs_by_coords_32bpp(coords, surface->res.x)] = color.rgb;
    return;
}

void srfc_fill(srfc_t* surface, color32bpp_t color)
{
    assert(surface->bpp == 32);

    for (unsigned iter = 0; iter < surface->res.x * surface->res.y; iter++)
    {
        surface->buffer[iter] = color.rgb;
    }

    return;
}

void srfc_clear(srfc_t* surface)
{
    color32bpp_t black = {.rgb = 0x000000};
    srfc_fill(surface, black);

    return;
}

void srfc_copy(srfc_t* ssurface, srfc_t* dsurface, pair16_t src, pair16_t dst, size_t count)
{
    assert(ssurface->bpp == 32);
    assert(dsurface->bpp == 32);

    assert(ssurface->res.x > src.x && ssurface->res.y > src.y);
    assert(dsurface->res.x > dst.x && dsurface->res.y > dst.y);

    size_t src_offs = offs_by_coords_32bpp(src, ssurface->res.x);
    size_t dst_offs = offs_by_coords_32bpp(dst, dsurface->res.x);

    assert(src_offs + count < ssurface->res.x * ssurface->res.y);
    assert(dst_offs + count < dsurface->res.x * dsurface->res.y);

    for (size_t iter = 0; iter < count; iter++)
    {
        dsurface->buffer[dst_offs + iter] = ssurface->buffer[src_offs + iter];
    }

    return;
}

void srfc_hzline(srfc_t* surface, uint16_t x1, uint16_t x2, uint16_t y, color32bpp_t color)
{
    assert(surface->bpp == 32);

    assert(x1 <= x2);
    assert(x2 < surface->res.x);
    assert(y  < surface->res.y);

    if (x1 == x2)
    {
        pair16_t point = {.x = x1, .y = y};
        srfc_set_pxl(surface, point, color);
        return;
    }

    pair16_t start = {.x = x1, .y = y};
    size_t start_offs = offs_by_coords_32bpp(start, surface->res.x);
    
    pair16_t end = {.x = x2, .y = y};
    size_t end_offs = offs_by_coords_32bpp(end, surface->res.x);

    while (start_offs <= end_offs)
    {
        surface->buffer[start_offs] = color.rgb;
        start_offs += 1;
    }

    return;
}

void srfc_vtline(srfc_t* surface, uint16_t y1, uint16_t y2, uint16_t x, color32bpp_t color)
{
    assert(surface->bpp == 32);

    assert(y1 <= y2);
    assert(y2 < surface->res.y);
    assert(x  < surface->res.x);

    if (y1 == y2)
    {
        pair16_t point = {.x = x, .y = y1};
        srfc_set_pxl(surface, point, color);
        return; 
    }

    pair16_t start = {.x = x, .y = y1};
    size_t start_offs = offs_by_coords_32bpp(start, surface->res.x);
    
    pair16_t end = {.x = x, .y = y2};
    size_t end_offs = offs_by_coords_32bpp(end, surface->res.x);

    while (start_offs <= end_offs)
    {
        surface->buffer[start_offs] = color.rgb;
        start_offs += surface->res.x;
    }

    return;
}

void srfc_line(srfc_t* surface, pair16_t start, pair16_t end, color32bpp_t color)
{
    assert(surface->bpp == 32);

    assert(start.x < surface->res.x && start.y < surface->res.y);
    assert(end.x < surface->res.x && end.y < surface->res.y);

    if (start.x == end.x)
    {
        uint16_t y1 = (start.y < end.y)? start.y : end.y;
        uint16_t y2 = (start.y > end.y)? start.y : end.y;

        srfc_vtline(surface, y1, y2, start.x, color);
        return;
    }

    if (start.y == end.y)
    {
        uint16_t x1 = (start.x < end.x)? start.x: end.x;
        uint16_t x2 = (start.x > end.x)? start.x: end.x;
    
        srfc_hzline(surface, x1, x2, start.y, color);
        return;
    }

    int16_t delta_x = end.x - start.x;
    int16_t delta_y = end.y - start.y;

    int16_t inc_x = (delta_x > 0)? 1 : -1;
    int16_t inc_y = (delta_y > 0)? surface->res.x : -surface->res.x;

    delta_x = ((delta_x > 0)? delta_x : -delta_x) + 1;
    delta_y = ((delta_y > 0)? delta_y : -delta_y) + 1;

    int16_t x, y, t;
    x = y = t = 0;
    int16_t dist = (delta_x > delta_y)? delta_x : delta_y;

    uint32_t* addr = surface->buffer + offs_by_coords_32bpp(start, surface->res.x);

    while (t <= dist)
    {
        *addr = color.rgb;

        x += delta_x;
        y += delta_y;

        if (x > dist)
        {
            x -= dist;
            addr += inc_x;
        }

        if (y > dist)
        {
            y -= dist;
            addr += inc_y;
        }

        t += 1;
    }

    return;
}

void srfc_box(srfc_t* surface, pair16_t bl, pair16_t tr, color32bpp_t color)
{
    assert(surface->bpp == 32);

    assert(bl.x <= tr.x);
    assert(bl.y <= tr.y);

    srfc_hzline(surface, bl.x, tr.x, tr.y, color);
    srfc_hzline(surface, bl.x, tr.x, bl.y, color);
    srfc_vtline(surface, tr.y, bl.y, bl.x, color);
    srfc_vtline(surface, tr.y, bl.y, tr.x, color);

    return;
}

void srfc_box_thick_in(srfc_t* surface, pair16_t bl, pair16_t tr, color32bpp_t color, uint16_t thickness)
{
    assert(surface->bpp == 32);

    assert(bl.x <= tr.x);
    assert(tr.y <= bl.y);

    /*
         ___C_________ ___tr
        |   |         |   |
        |___|_________|___|
        |A  |         |   | 
        |   |         |   |
        |___|_________|___|
        |   |         |   |B
        |___|_________|___|
        bl            D    
    */

    pair16_t pA = {.x = bl.x, .y = tr.y + thickness};
    pair16_t pB = {.x = tr.x, .y = bl.y - thickness};
    pair16_t pC = {.x = bl.x + thickness, .y = tr.y};
    pair16_t pD = {.x = tr.x - thickness, .y = bl.y}; 

    srfc_bar(surface, pA, tr, color);
    srfc_bar(surface, bl, pB, color);
    srfc_bar(surface, pD, tr, color);
    srfc_bar(surface, bl, pC, color);

    return;
}

void srfc_bar(srfc_t* surface, pair16_t bl, pair16_t tr, color32bpp_t color)
{
    assert(surface->bpp == 32);

    assert(bl.x <= tr.x);
    assert(tr.y <= bl.y);

    uint16_t start_y = tr.y;
    uint16_t end_y   = bl.y;

    while (start_y <= end_y)
    {
        srfc_hzline(surface, bl.x, tr.x, start_y, color);
        start_y += 1;
    }

    return;
}

static uint32_t distance2(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    return ((uint32_t) ((x1 - x2) * (x1 - x2)) + (uint32_t)((y1 - y2) * (y1 - y2)));
}

static const float Surface_cf_eps            = 0.05f;
static const float Surface_cf_eps_small      = 0.1f;
static const float Surface_cf_eps_very_small = 0.15f;

void srfc_cf(srfc_t* surface, pair16_t center, uint16_t rad, color32bpp_t color)
{
    assert(surface->bpp == 32);
    assert(center.x < surface->res.x && center.y < surface->res.y);

    if (rad == 1)
    {
        srfc_set_pxl(surface, center, color);
        return;
    }
    else if (rad == 0)
        return;

    int16_t x0 = (int16_t) center.x;
    int16_t y0 = (int16_t) center.y;

    int16_t start_x = x0 - (int16_t) rad;
    int16_t start_y = y0 - (int16_t) rad;

    int16_t end_x = x0 + (int16_t) rad;
    int16_t end_y = y0 + (int16_t) rad;

    int16_t x = start_x;
    int16_t y = start_y;

    uint32_t rad2 = rad * rad;

    float cf_eps = (rad <= 20)? Surface_cf_eps_very_small : (rad <= 50)? Surface_cf_eps_small : Surface_cf_eps;

    uint32_t rad2_eps = rad2 - (uint32_t) (cf_eps * rad2);

    while (x <= end_x && y <= end_y)
    {
        if (x < 0 || x >= surface->res.x)
            goto next;

        if (y < 0 || y >= surface->res.y)
            goto next;

        uint32_t dist2 = distance2(x, y, x0, y0);

        if (dist2 > rad2_eps && dist2 < rad2)
        {
            pair16_t point = {.x = x, .y = y};
            srfc_set_pxl(surface, point, color);
        }

    next:
        x += 1;

        if (x >= end_x)
        {
            x = start_x;
            y += 1;
        }
    }

    return;
}

void srfc_circle(srfc_t* surface, pair16_t center, uint16_t rad, color32bpp_t color)
{
    assert(surface->bpp == 32);
    assert(center.x < surface->res.x && center.y < surface->res.y);

    if (rad == 1)
    {
        srfc_set_pxl(surface, center, color);
        return;
    }
    else if (rad == 0)
        return;

    int16_t x0 = center.x;
    int16_t y0 = center.y;

    int16_t x = x0 - rad;
    int16_t y = y0 - rad;

    int16_t end_x = x0 + rad;
    int16_t end_y = y0 + rad;

    uint32_t rad2 = rad * rad;

    while (x <= end_x && y <= end_y)
    {
        if (x < 0 || x >= surface->res.x)
            goto next;

        if (y < 0 || y >= surface->res.y)
            goto next;

        if (distance2(x, y, x0, y0) <= rad2)
        {
            pair16_t point = {.x = x, .y = y};
            srfc_set_pxl(surface, point, color);
        }

    next:
        x += 1;

        if (x > end_x)
        {
            x = x0 - rad;
            y += 1;
        }
    }

    return;
}

void srfc_cf_thick_in(srfc_t* surface, pair16_t center, uint16_t rad, uint16_t thickness, color32bpp_t color)
{
    assert(surface->bpp == 32);
    assert(center.x < surface->res.x && center.y < surface->res.y);

    if (thickness > rad) 
        thickness = rad;

    uint16_t rad_in = rad - thickness;
    while (rad >= rad_in)
    {
        srfc_cf(surface, center, rad, color);
        rad -= 1;
    }

    return;
}