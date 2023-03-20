#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/gpu.h>
#include <inc/string.h>
#include <kern/graphics.h>

extern void map_addr_early_boot(uintptr_t va, uintptr_t pa, size_t sz);

static vga_dev_t Vga_dev = { 0 };

static int vga_find_pci(void);
static int vga_map_memory(void);
static int vga_check_bga_avail(void);
static int vga_set_display_pref(void);
static int vga_set_virtual_display_pref(void);

static int gpu_set_display_vres(pair16_t vres);

void init_gpu(void)
{
    if (trace_gpu)
        cprintf("vga: initialization stated. \n");

    int err = 0;

    err = vga_find_pci();
    if (err < 0) goto panic;

    err = vga_map_memory();
    if (err < 0) goto panic;

    err = vga_check_bga_avail();
    if (err < 0) goto panic;

    err = vga_set_display_pref();
    if (err < 0) goto panic;

    Vga_dev.srfc_requested = false;
    Vga_dev.srfc_submitted = false;

    Vga_dev.initialized = true;

    if (trace_gpu)
        cprintf("vga: initialization finished. \n");

    return;

panic:
    panic("vga: initialization failed: %i \n", err);
}

static int vga_map_memory()
{
    if (trace_gpu)
        cprintf("vga: mapping memory. \n");

    if (Vga_dev.pci_dev_general.bar_addr[Fb_barn].type != BAR_MEMORY_32
     || Vga_dev.pci_dev_general.bar_addr[Fb_barn].memory == 0
     || Vga_dev.pci_dev_general.bar_addr[MMIO_barn].type != BAR_MEMORY_32
     || Vga_dev.pci_dev_general.bar_addr[MMIO_barn].memory == 0)
    {
        cprintf("vga: invalid BARs' types or memory addresses. \n");
        return -E_INVAL;
    }

    Vga_dev.fb   = (uint32_t*) Vga_dev.pci_dev_general.bar_addr[Fb_barn].memory;
    Vga_dev.mmio = (void*) Vga_dev.pci_dev_general.bar_addr[MMIO_barn].memory;

    map_addr_early_boot((uintptr_t) Vga_dev.fb, (uintptr_t) Vga_dev.fb, Fb_size);
    Vga_dev.mmio = (void*) mmio_map_region((uintptr_t) Vga_dev.mmio, MMIO_size);

    if (trace_gpu)
        cprintf("vga: memory mapped. \n");
    return 0;
}

static int vga_set_display_pref(void)
{
    if (trace_gpu)
        cprintf("vga: setting up display res and bpp: (Xres;Yres) = (%u;%u) BPP=%u. \n", Display_start_res.x, 
                                                                                         Display_start_res.y, 
                                                                                         Display_bpp);

    vbe_dispi_disable();
    vbe_dispi_set_reg(VBE_DISPI_INDEX_XRES, Display_start_res.x);
    vbe_dispi_set_reg(VBE_DISPI_INDEX_YRES, Display_start_res.y);
    vbe_dispi_set_reg(VBE_DISPI_INDEX_BPP, Display_bpp);
    vbe_dispi_enable(VBE_DISPI_LFB_ENABLED);

    if (!vbe_dispi_check_bit(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_LFB_ENABLED))
    {
        cprintf("vga: failed to turn on necessary features. \n");
        return -E_DEV_RT;
    }

    uint16_t xres_set = vbe_dispi_get_reg(VBE_DISPI_INDEX_XRES); 
    uint16_t yres_set = vbe_dispi_get_reg(VBE_DISPI_INDEX_YRES); 
    uint16_t bpp_set  = vbe_dispi_get_reg(VBE_DISPI_INDEX_BPP);

    if (xres_set != Display_start_res.x 
     || yres_set != Display_start_res.y 
     || bpp_set  != Display_bpp)
    {
        cprintf("vga: failed to set res & bpp. \n");
        return -E_DEV_RT;
    }

    Vga_dev.flags = VBE_DISPI_LFB_ENABLED;

    Vga_dev.res = Display_start_res;
    Vga_dev.bpp  = Display_bpp;

    int err = gpu_set_display_vres(Virt_display_start_res);
    if (err < 0) return err;

    if (trace_gpu)
        cprintf("vga: display res & bpp successfully initialized. \n");
    return 0;
}

static int vga_check_bga_avail(void)
{
    if (trace_gpu)
        cprintf("vga: checking if bga is available. \n");

    uint16_t bga_version = vbe_dispi_get_reg(VBE_DISPI_INDEX_ID);
    if (bga_version != VBE_DISPI_ID5)
    {
        if (trace_gpu)
            cprintf("vga: requesting desired bga version. ");

        vbe_dispi_set_reg(VBE_DISPI_INDEX_ID, VBE_DISPI_ID5);
        bga_version = vbe_dispi_get_reg(VBE_DISPI_INDEX_ID);

        if (bga_version != VBE_DISPI_ID5)
        {
            cprintf("vga: bga's version is not supported. \n");
            return -E_DEV_RT;
        }
    }

    if (trace_gpu)
        cprintf("vga: bga is available. \n");
    return 0;
}

static int vga_find_pci()
{
    if (trace_gpu)
        cprintf("vga: performing search for vga over PCI. \n");

    int err = pci_dev_find(VGA_TO_PCI(&Vga_dev),
                           Vga_pci_class,
                           Vga_pci_subclass,
                           Vga_pci_vendor_id);
    if (err < 0)
    {
        cprintf("vga: failed to find device over PCI. \n");
        return err;
    }

    if (VGA_TO_PCI(&Vga_dev)->header_type != GENERAL_DEVICE)
    {
        cprintf("vga: invalid pci device header type: expected = %d, header_type = %d\n", 
                                                                          GENERAL_DEVICE, 
                                                       VGA_TO_PCI(&Vga_dev)->header_type);
        return -E_INVAL;
    }

    err = pci_dev_general_read_header(VGA_TO_PCI_GENERAL(&Vga_dev));
    if (err < 0)
    {
        cprintf("vga: failed to read GENERAL_DEVICE header. \n");
        return err;
    }

    if (trace_gpu)
    {
        dump_pci_dev_general(VGA_TO_PCI_GENERAL(&Vga_dev));
        pci_dev_dump_cmnd_reg(VGA_TO_PCI(&Vga_dev));
        pci_dev_dump_stat_reg(VGA_TO_PCI(&Vga_dev));
        pci_dev_general_dump_bars(VGA_TO_PCI_GENERAL(&Vga_dev));
    }

    if (VGA_TO_PCI(&Vga_dev)->device_id != Vga_pci_device_id)
    {
        cprintf("vga: invalid device_id: expected = %x, device_id = %x\n", 
                                                        Vga_pci_device_id,
                                          VGA_TO_PCI(&Vga_dev)->device_id);
        return -E_INVAL;
    }

    return 0;
}

void test_gpu(void)
{
    if (trace_gpu)
        cprintf("vga: tests started. \n");
    
    int16_t rad = 16;
    int16_t rad_inc = 1;

    while (1)
    {
        srfc_t surface = { 0 };
        int err = gpu_request_surface(&surface);
        assert(err == 0);

        srfc_clear(&surface);

        pair16_t tl = {.x = 0, .y = 0};
        // pair16_t br = {.x = Vga_dev.res.x - 1, .y = Vga_dev.res.y - 1};

        pair16_t tr = {.x = Vga_dev.res.x - 1, .y = 0}; 
        pair16_t bl = {.x = 0, .y = Vga_dev.res.y - 1};

        color32bpp_t lc = {.rgb = 0xFF0000}; 

        // srfc_line(&surface, tl, br, lc);
        // srfc_line(&surface, tr, bl, lc);

        // srfc_hzline(&surface, 0, Vga_dev.res.x - 1, Vga_dev.res.y / 2, lc);
        // srfc_vtline(&surface, 0, Vga_dev.res.y - 1, Vga_dev.res.x / 2, lc);

        srfc_box_thick_in(&surface, bl, tr, lc, 16);

        pair16_t center = {.x = Vga_dev.res.x / 2, .y = Vga_dev.res.y / 2};
        srfc_cf(&surface, center, rad, lc);

        pair16_t p1 = {.x = center.x - 12, .y = center.y + 12};
        pair16_t p2 = {.x = center.x + 12, .y = center.y - 12};
        
        color32bpp_t pink  = {.rgb = 0x00FF4EAB};
        color32bpp_t white = {.rgb = 0x00FFFFFF};

        srfc_bar(&surface, p1, p2, pink);
        srfc_puts(&surface, "Teper u menya est' pervoye", tl, white, 10);

        err = gpu_submit_surface(&surface);
        assert(err == 0);

        err = gpu_page_flip();
        assert(err == 0);

        rad += rad_inc;

        if (rad == 0) rad_inc = 1;
        if (rad == 50) rad_inc = -1;

        int ct = 100000000;
        while (ct-- != 0);
    }

    if (trace_gpu)
        cprintf("vga: tests finished. \n");

    return;
}

pair16_t gpu_get_display_res(void)
{
    return Vga_dev.res;
}

static int gpu_set_display_vres(pair16_t vres)
{
    if (trace_gpu)
        cprintf("vga: setting up new virtual res: (%u;%u) \n", vres.x, vres.y);

    vbe_dispi_set_reg(VBE_DISPI_INDEX_VIRT_WIDTH, vres.x);

    if (vbe_dispi_get_reg(VBE_DISPI_INDEX_VIRT_HEIGHT) != vres.y)
    {
        cprintf("vga: failed to initialize virtual display \n");
        return -E_DEV_RT;
    }

    vbe_dispi_set_reg(VBE_DISPI_INDEX_X_OFFSET, 0);
    vbe_dispi_set_reg(VBE_DISPI_INDEX_Y_OFFSET, 0);

    Vga_dev.vres = vres;
    Vga_dev.voffs.x = 0; 
    Vga_dev.voffs.y = 0; 

    Vga_dev.display2coords.x = 0;
    Vga_dev.display2coords.y = Vga_dev.res.y;
    Vga_dev.display2offs = vidmem_offset_by_coords(Vga_dev.display2coords, Vga_dev.res.x);

    return 0;
}

int gpu_set_display_res(pair16_t res)
{
    if (trace_gpu)
        cprintf("vga: setting up new res: (%u;%u) \n", res.x, res.y);

    if (res.x > Display_max_res.x || res.y > Display_max_res.y)
        return -E_INVAL;

    vbe_dispi_disable();
    vbe_dispi_set_reg(VBE_DISPI_INDEX_XRES, Display_start_res.x);
    vbe_dispi_set_reg(VBE_DISPI_INDEX_YRES, Display_start_res.y);
    vbe_dispi_enable(Vga_dev.flags);

    if (!vbe_dispi_check_bits(VBE_DISPI_INDEX_ENABLE, Vga_dev.flags))
    {
        cprintf("vga: failed to turn on necessary features. \n");
        return -E_DEV_RT;
    }

    uint16_t xres_set = vbe_dispi_get_reg(VBE_DISPI_INDEX_XRES); 
    uint16_t yres_set = vbe_dispi_get_reg(VBE_DISPI_INDEX_YRES); 

    if (xres_set != Display_start_res.x || yres_set != Display_start_res.y)
    {
        cprintf("vga: failed to set res. \n");
        return -E_DEV_RT;
    }

    pair16_t vres = {.x = res.x, .y = Fb_size / (sizeof(uint32_t) * res.x)};
    
    return gpu_set_display_vres(vres);
}

int gpu_page_flip(void)
{
    if (Vga_dev.srfc_submitted != true)
        return -1;

    vbe_dispi_set_reg(VBE_DISPI_INDEX_X_OFFSET, Vga_dev.display2coords.x);
    vbe_dispi_set_reg(VBE_DISPI_INDEX_Y_OFFSET, Vga_dev.display2coords.y);

    if (Vga_dev.display2coords.y != 0)
    {
        Vga_dev.display2coords.x = 0;
        Vga_dev.display2coords.y = Vga_dev.res.y;
    }
    else 
    {
        Vga_dev.display2coords.x = 0;
        Vga_dev.display2coords.y = 0;
    }

    Vga_dev.display2offs = vidmem_offset_by_coords(Vga_dev.display2coords, Vga_dev.res.x);

    return 0;
}

int gpu_request_surface(srfc_t* surface)
{
    if (Vga_dev.srfc_requested != false)
        return -1;

    surface->res = Vga_dev.res;
    surface->buffer = Vga_dev.fb + vidmem_offset_by_coords(Vga_dev.display2coords, Vga_dev.res.x);
    surface->bpp = Vga_dev.bpp;

    Vga_dev.srfc_requested = true;
    Vga_dev.srfc_submitted = false;

    return 0;
}

int gpu_submit_surface(const srfc_t* surface)
{
    if (Vga_dev.srfc_requested != true || Vga_dev.srfc_submitted != false)
        return -1;

    if (surface->buffer != Vga_dev.fb + vidmem_offset_by_coords(Vga_dev.display2coords, Vga_dev.res.x))
        return -E_INVAL;

    if (surface->res.x != Vga_dev.res.x 
     || surface->res.y != Vga_dev.res.y)
        return -E_INVAL;

    if (surface->bpp != Vga_dev.bpp)
        return -E_INVAL;

    Vga_dev.srfc_requested = false;
    Vga_dev.srfc_submitted = true;

    return 0;
}

void gpu_clear_display(void)
{
    vbe_dispi_disable();
    vbe_dispi_enable(Vga_dev.flags);
}

bool gpu_ready(void)
{
    return Vga_dev.initialized;
}
