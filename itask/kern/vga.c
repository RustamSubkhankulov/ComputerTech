#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/vga.h>
#include <inc/string.h>

static vga_dev_t Vga_dev = { 0 };

static int vga_find_pci();

extern void map_addr_early_boot(uintptr_t va, uintptr_t pa, size_t sz);

void init_vga(void)
{
    if (trace_vga)
        cprintf("vga: initialization stated. \n");

    int err = vga_find_pci();
    if (err < 0)
    {
        cprintf("vga: failed to find/read vga device over pci. \n");
        goto panic;
    }

    if (Vga_dev.pci_dev_general.bar_addr[Fb_barn].type != BAR_MEMORY_32
     || Vga_dev.pci_dev_general.bar_addr[Fb_barn].memory == 0
     || Vga_dev.pci_dev_general.bar_addr[MMIO_barn].type != BAR_MEMORY_32
     || Vga_dev.pci_dev_general.bar_addr[MMIO_barn].memory == 0)
    {
        cprintf("vga: invalid BARs' types or memory addresses \n");
        goto panic;
    }

    Vga_dev.fb   = Vga_dev.pci_dev_general.bar_addr[Fb_barn].memory;
    Vga_dev.mmio = Vga_dev.pci_dev_general.bar_addr[MMIO_barn].memory;

    if (trace_vga)
    {
        pci_dev_dump_cmnd_reg(VGA_TO_PCI(&Vga_dev));
        pci_dev_dump_stat_reg(VGA_TO_PCI(&Vga_dev));
        pci_dev_general_dump_bars(VGA_TO_PCI_GENERAL(&Vga_dev));
    }

    map_addr_early_boot(Vga_dev.fb, Vga_dev.fb, Fb_size);
    Vga_dev.mmio = (uint64_t) mmio_map_region(Vga_dev.mmio, MMIO_size);

    if (trace_vga)
        cprintf("vga: initialization finished. \n");

    return;

panic:
    panic("vga: initialization failed \n");
}

static int vga_find_pci()
{
    if (trace_vga)
        cprintf("vga: performing search for vga over PCI \n");

    int err = pci_dev_find(VGA_TO_PCI(&Vga_dev),
                           Vga_pci_class,
                           Vga_pci_subclass,
                           Vga_pci_vendor_id);
    if (err < 0)
    {
        cprintf("vga: failed to find device over PCI \n");
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
        cprintf("vga: failed to read GENERAL_DEVICE header \n");
        return err;
    }

    if (trace_vga)
        dump_pci_dev_general(VGA_TO_PCI_GENERAL(&Vga_dev));

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
        cprintf("vga: tests started \n");

    if (trace_vga)
        cprintf("vga: tests finished \n");

    return;
}
