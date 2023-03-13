#ifndef JOS_KERN_VGA_H
#define JOS_KERN_VGA_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/vga_regs.h>
#include <kern/vbe.h>
#include <kern/standard_vga.h>

static const uint16_t Display_res_x = 1024U;
static const uint16_t Display_res_y = 768U;

static const uint16_t Virt_display_res_x = 1024U;
static const uint16_t Virt_display_res_y = Fb_size / (sizeof(uint32_t) * Virt_display_res_x);

static const enum Vbe_dispi_bpp Display_bpp  = VBE_DISPI_BPP_32;

typedef struct Vga_dev
{
    pci_dev_general_t pci_dev_general;
    
    uint64_t fb;
    uint64_t mmio;

    uint16_t xres;
    uint16_t yres;
    uint16_t bpp;

    uint16_t vxres;
    uint16_t vyres;
    uint16_t x_offs;
    uint16_t y_offs;

    bool dac_8bit;
    bool lfb_enabled;
    bool banked_mode_enabled;
    bool noclearmem_enabled;

} vga_dev_t;

void init_vga(void);
void test_vga(void);

static inline size_t vidmem_offset_by_coords(uint16_t x, uint16_t y, uint16_t xres)
{   
    return (size_t) ((y * xres + x) * sizeof(uint32_t));
}

#define VGA_TO_PCI_GENERAL(vga_dev) (&((vga_dev)->pci_dev_general))
#define VGA_TO_PCI(vga_dev) (&((vga_dev)->pci_dev_general.pci_dev))

#endif // JOS_KERN_VGA_H