#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/vga.h>
#include <inc/string.h>

extern void map_addr_early_boot(uintptr_t va, uintptr_t pa, size_t sz);

static vga_dev_t Vga_dev = { 0 };

static int vga_find_pci(void);
static int vga_map_memory(void);
static int vga_check_bga_avail(void);
static int vga_set_display_pref(void);
static int vga_set_virtual_display_pref(void);

void init_vga(void)
{
    if (trace_vga)
        cprintf("vga: initialization stated. \n");

    int err = vga_find_pci();
    if (err < 0) goto panic;

    err = vga_map_memory();
    if (err < 0) goto panic;

    err = vga_check_bga_avail();
    if (err < 0) goto panic;

    err = vga_set_display_pref();
    if (err < 0) goto panic;

    err = vga_set_virtual_display_pref();
    if (err < 0) goto panic;

    if (trace_vga)
        cprintf("vga: initialization finished. \n");

    return;

panic:
    panic("vga: initialization failed. \n");
}

static int vga_map_memory()
{
    if (trace_vga)
        cprintf("vga: mapping memory. \n");

    if (Vga_dev.pci_dev_general.bar_addr[Fb_barn].type != BAR_MEMORY_32
     || Vga_dev.pci_dev_general.bar_addr[Fb_barn].memory == 0
     || Vga_dev.pci_dev_general.bar_addr[MMIO_barn].type != BAR_MEMORY_32
     || Vga_dev.pci_dev_general.bar_addr[MMIO_barn].memory == 0)
    {
        cprintf("vga: invalid BARs' types or memory addresses. \n");
        return -1;
    }

    Vga_dev.fb   = Vga_dev.pci_dev_general.bar_addr[Fb_barn].memory;
    Vga_dev.mmio = Vga_dev.pci_dev_general.bar_addr[MMIO_barn].memory;

    map_addr_early_boot(Vga_dev.fb, Vga_dev.fb, Fb_size);
    Vga_dev.mmio = (uint64_t) mmio_map_region(Vga_dev.mmio, MMIO_size);

    if (trace_vga)
        cprintf("vga: memory mapped. \n");
    return 0;
}

static int vga_set_display_pref(void)
{
    if (trace_vga)
        cprintf("vga: setting up display res and bpp: (Xres;Yres) = (%u;%u) BPP=%u. \n", Display_res_x, 
                                                                                         Display_res_y, 
                                                                                         Display_bpp);

    vbe_dispi_disable();
    vbe_dispi_set_reg(VBE_DISPI_INDEX_XRES, Display_res_x);
    vbe_dispi_set_reg(VBE_DISPI_INDEX_YRES, Display_res_y);
    vbe_dispi_set_reg(VBE_DISPI_INDEX_BPP, Display_bpp);
    vbe_dispi_enable(VBE_DISPI_LFB_ENABLED);

    assert(vbe_dispi_check_bit(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_LFB_ENABLED));

    uint16_t xres_set = vbe_dispi_get_reg(VBE_DISPI_INDEX_XRES); 
    uint16_t yres_set = vbe_dispi_get_reg(VBE_DISPI_INDEX_YRES); 
    uint16_t bpp_set  = vbe_dispi_get_reg(VBE_DISPI_INDEX_BPP);

    if (xres_set != Display_res_x 
     || yres_set != Display_res_y 
     || bpp_set  != Display_bpp)
    {
        cprintf("vga: failed to set res & bpp. \n");
        return -1;
    }

    Vga_dev.dac_8bit = false;
    Vga_dev.noclearmem_enabled = false;

    Vga_dev.lfb_enabled = true;
    Vga_dev.banked_mode_enabled = false;

    Vga_dev.xres = Display_res_x;
    Vga_dev.yres = Display_res_y;
    Vga_dev.bpp  = Display_bpp;

    if (trace_vga)
        cprintf("vga: display res & bpp successfully initialized. \n");
    return 0;
}

static int vga_set_virtual_display_pref(void)
{
    if (trace_vga)
        cprintf("vga: setting up virtual display preferences: (Xres;Yres) = (%u;%u). \n", Virt_display_res_x,
                                                                                          Virt_display_res_y);

    vbe_dispi_set_reg(VBE_DISPI_INDEX_VIRT_WIDTH, Virt_display_res_x);

    if (vbe_dispi_get_reg(VBE_DISPI_INDEX_VIRT_HEIGHT) != Virt_display_res_y)
    {
        cprintf("vga: faile to initialize virtual display \n");
        return -1;
    }

    vbe_dispi_set_reg(VBE_DISPI_INDEX_X_OFFSET, 0);
    vbe_dispi_set_reg(VBE_DISPI_INDEX_Y_OFFSET, 0);

    Vga_dev.vxres  = Virt_display_res_x; 
    Vga_dev.vyres  = Virt_display_res_y;
    Vga_dev.x_offs = 0;
    Vga_dev.y_offs = 0;

    if (trace_vga)
        cprintf("vga: virtual display res successfully initialized. \n");
    return 0;
}

static int vga_check_bga_avail(void)
{
    if (trace_vga)
        cprintf("vga: checking if bga is available. \n");

    uint16_t bga_version = vbe_dispi_get_reg(VBE_DISPI_INDEX_ID);
    if (bga_version != VBE_DISPI_ID5)
    {
        if (trace_vga)
            cprintf("vga: requesting desired bga version. ");

        vbe_dispi_set_reg(VBE_DISPI_INDEX_ID, VBE_DISPI_ID5);
        bga_version = vbe_dispi_get_reg(VBE_DISPI_INDEX_ID);

        if (bga_version != VBE_DISPI_ID5)
        {
            cprintf("vga: bga's version is not supported. \n");
            return -1;
        }
    }

    if (trace_vga)
        cprintf("vga: bga is available. \n");
    return 0;
}

static int vga_find_pci()
{
    if (trace_vga)
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
        return -1;
    }

    err = pci_dev_general_read_header(VGA_TO_PCI_GENERAL(&Vga_dev));
    if (err < 0)
    {
        cprintf("vga: failed to read GENERAL_DEVICE header. \n");
        return err;
    }

    if (trace_vga)
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
        return -1;
    }

    return 0;
}

void test_vga(void)
{
    if (trace_vga)
        cprintf("vga: tests started. \n");

    if (trace_vga)
        cprintf("vga: tests finished. \n");

    return;
}
