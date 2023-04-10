#ifndef JOS_KERN_VBE_H
#define JOS_KERN_VBE_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/standard_vga.h>

#define VBE_DISPI_INDEX_PORT    0x1CE

// Temporary here
#define ARCH_X86

#ifdef ARCH_X86
    #define VBE_DISPI_DATA_PORT 0x1CF
#else 
    #define VBE_DISPI_DATA_PORT 0x1D0
#endif 

#define VBE_DISPI_MAX_XRES 16000
#define VBE_DISPI_MAX_YRES 12000
#define VBE_DISPI_MAX_BPP  32

/*

Displaying GFX (banked mode)
--------------
  What happens is that the total screen is devided in banks of 'VBE_DISPI_BANK_SIZE_KB' KiloByte in size.
  If you want to set a pixel you can calculate its bank by doing:

    offset = pixel_x + pixel_y * resolution_x;
    bank = offset / 64 Kb (rounded 1.9999 -> 1)

    bank_pixel_pos = offset - bank * 64Kb

  Now you can set the current bank and put the pixel at VBE_DISPI_BANK_ADDRESS + bank_pixel_pos

*/

#define VBE_DISPI_BANK_SIZE_KB (64 * KB)
#define VBE_DISPI_BANK_ADDRESS 0xA0000;

#define VBE_DISPI_LFB_PHYSICAL_ADDRESS 0xE0000000

enum Vbe_dispi_reg
{
    VBE_DISPI_INDEX_ID          = 0x00, // r/w 
    VBE_DISPI_INDEX_XRES        = 0x01, // r/w VBE must be enabled before changing
    VBE_DISPI_INDEX_YRES        = 0x02, // r/w VBE must be enabled before changing
    VBE_DISPI_INDEX_BPP         = 0x03, // r/w VBE must be enabled before changing
    VBE_DISPI_INDEX_ENABLE      = 0x04, // r/w 
    VBE_DISPI_INDEX_BANK        = 0x05, // r/w 
    VBE_DISPI_INDEX_VIRT_WIDTH  = 0x06, // r/w 
    VBE_DISPI_INDEX_VIRT_HEIGHT = 0x07, // r/- 
    VBE_DISPI_INDEX_X_OFFSET    = 0x08, // r/w 
    VBE_DISPI_INDEX_Y_OFFSET    = 0x09  // r/w 
};

/*
#define VBE_DISPI_INDEX_NB               0x0A // size of vbe_regs[] 
#define VBE_DISPI_INDEX_VIDEO_MEMORY_64K 0x0A // read-only, not in vbe_regs
*/

enum Vbe_dispi_bpp
{
    VBE_DISPI_BPP_4  = 0x04,
    VBE_DISPI_BPP_8  = 0x08,
    VBE_DISPI_BPP_15 = 0x0F,
    VBE_DISPI_BPP_16 = 0x10,
    VBE_DISPI_BPP_24 = 0x18,
    VBE_DISPI_BPP_32 = 0x20
};

/*

BPP explained:

BPP4:

 clrbit 0 clrbit 1 clrbit 2  clrbit 0 clrbit 1 clrbit 2 ...
 ________ ________ ________ ________ ________ ________ ____
|________|________|________|________|________|________|____...
p       p p      p p      p p      p p      p p      p p   
i       i i      i i      i i      i i      i i      i i   
x  ...  x x  ... x x  ... x x  ... x x  ... x x  ... x x   ...
e       e e      e e      e e      e e      e e      e e   
l       l l      l l      l l      l l      l l      l l   
0       7 0      7 0      7 0      7 0      7 0      7 7-   

BPP8 - each pixel is exactly one byte

BPP15:

 1   5     5     5   1   5     5     5   1   5     5     5    
 _ _____ _____ _____ _ _____ _____ _____ _ _____ _____ _____  
|_|_____|_____|_____|_|_____|_____|_____|_|_____|_____|_____| 
 i   r     g     b   i   r     g     b   i   r     g     b    
 g   e     r     l   g   e     r     l   g   e     r     l    
 n   d     e     u   n   d     e     u   n   d     e     u    ...
 o         e     e   o         e     e   o         e     e    
 r         n         r         n         r         n          
 e                   e                   e                    
 d                   d                   d                    
                    
|___________________|___________________|____________________|
       Pixel 0             Pixel 1               Pixel 2           

BPP16:

   5      6     5      5      6     5      5      6     5   
 _____ ______ _____  _____ ______ _____  _____ ______ _____ 
|_____|______|_____||_____|______|_____||_____|______|_____|...
   r      g     b      r      g     b      r      g     b   

|__________________||__________________||__________________|
        Pixel 0             Pixel 1            Pixel 2

BPP24:
 __________________________ 
|    One pixel - 3 bytes   |
 ________ ________ ________ 
|________|________|________|
    B         G        R    

BPP32:

Accessing pixels as longwords the colour should 
be defined as 0x00RRGGBB.

 ___________________________________ 
|        One pixel - 4 bytes        |
 ________ ________ ________ ________
|________|________|________|________|
    B         G        R    Ignored

*/

typedef union Color32bpp
{
    struct 
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };

    uint32_t rgb;
} color32bpp_t;

enum Vbe_dispi_index_id
{
    VBE_DISPI_ID0 = 0xB0C0,
    VBE_DISPI_ID1 = 0xB0C1,
    VBE_DISPI_ID2 = 0xB0C2,
    VBE_DISPI_ID3 = 0xB0C3,
    VBE_DISPI_ID4 = 0xB0C4,
    VBE_DISPI_ID5 = 0xB0C5
};

enum Vbe_dispi_index_enable_flag
{
    VBE_DISPI_DISABLED    = 0x00,
    VBE_DISPI_ENABLED     = 0x01,
    VBE_DISPI_GETCAPS     = 0x02,
    VBE_DISPI_8BIT_DAC    = 0x20,
    VBE_DISPI_LFB_ENABLED = 0x40,
    VBE_DISPI_NOCLEARMEM  = 0x80 // vidmem will not be cleared on enabling 
};

void     vbe_dispi_set_reg(enum Vbe_dispi_reg reg, uint16_t value);
uint16_t vbe_dispi_get_reg(enum Vbe_dispi_reg reg); 

bool vbe_dispi_check_bit(enum Vbe_dispi_reg reg, uint16_t bit);
void vbe_dispi_clear_bit(enum Vbe_dispi_reg reg, uint16_t bit);
void vbe_dispi_set_bit  (enum Vbe_dispi_reg reg, uint16_t bit);

bool vbe_dispi_check_bits(enum Vbe_dispi_reg reg, uint16_t bits);
void vbe_dispi_clear_bits(enum Vbe_dispi_reg reg, uint16_t bits);
void vbe_dispi_set_bits  (enum Vbe_dispi_reg reg, uint16_t bits);

static inline void vbe_dispi_enable(uint16_t flags)
{
    vbe_dispi_set_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | flags);
}

static inline void vbe_dispi_disable(void)
{
    vbe_dispi_set_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
}

#endif // JOS_KERN_VBE_H