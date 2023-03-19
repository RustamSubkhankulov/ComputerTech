#ifndef JOS_KERN_VGA_H
#define JOS_KERN_VGA_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/graphics.h>

#include <kern/vga_regs.h>
#include <kern/vbe.h>
#include <kern/standard_vga.h>

static const pair16_t Display_start_res      = {.x = 1024U, .y = 768U};
static const pair16_t Display_max_res        = {.x = 1920U, .y = 1080U};
static const pair16_t Virt_display_start_res = {.x = Display_start_res.x, .y = Fb_size / (sizeof(uint32_t) * Display_start_res.x)};

static const enum Vbe_dispi_bpp Display_bpp  = VBE_DISPI_BPP_32;

typedef struct Vga_dev
{
    pci_dev_general_t pci_dev_general;
    
    uint32_t* fb;
    void* mmio;

    pair16_t res;
    uint16_t bpp;

    pair16_t vres;
    pair16_t voffs;

    pair16_t display2coords;
    size_t display2offs;

    uint16_t flags;

    bool srfc_requested;
    bool srfc_submitted;

} vga_dev_t;

void init_gpu(void);
void test_gpu(void);

pair16_t gpu_get_display_res(void);
int gpu_set_display_res(pair16_t res);

static inline size_t vidmem_offset_by_coords(pair16_t coords, uint16_t xres)
{   
    return (size_t) ((coords.y * xres + coords.x));
}

void gpu_clear_display(void);
int gpu_page_flip(void);
int gpu_request_surface(srfc_t* surface);
int gpu_submit_surface(const srfc_t* surface);

#define VGA_TO_PCI_GENERAL(vga_dev) (&((vga_dev)->pci_dev_general))
#define VGA_TO_PCI(vga_dev) (&((vga_dev)->pci_dev_general.pci_dev))

#endif // JOS_KERN_VGA_H