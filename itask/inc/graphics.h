#ifndef JOS_INC_GRAPHICS_H
#define JOS_INC_GRAPHICS_H 1

#include <stdint.h>

typedef struct Pair8
{
    uint16_t x;
    uint16_t y;

} pair8_t;

typedef struct Pair16
{
    uint16_t x;
    uint16_t y;

} pair16_t;

typedef struct Pair32
{
    uint16_t x;
    uint16_t y;

} pair32_t;

typedef struct Pair64
{
    uint16_t x;
    uint16_t y;

} pair64_t;

typedef struct Surface
{
    uint32_t* buffer;

    pair16_t res;
    uint16_t bpp;

} srfc_t;

#endif // JOS_INC_GRAPHICS_H