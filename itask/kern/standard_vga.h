#ifndef JOS_KERN_STANDARD_VGA_H
#define JOS_KERN_STANDARD_VGA_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <kern/pci.h>
#include <kern/pmap.h>

enum Pci_display_controller_subclass
{
    PCI_SUBCLASS_VGA_COMPATIBLE_CONTROLLER = 0x0,
    PCI_SUBCLASS_XGA_CONTROLLER            = 0x1,
    PCI_SUBCLASS_3D_CONTROLLER             = 0x2,
    PCI_SUBCLASS_OTHER                     = 0x80
};

const static enum Pci_class 
                  Vga_pci_class = PCI_CLASS_DISPLAY_CONTROLLER;

const static enum Pci_display_controller_subclass 
                  Vga_pci_subclass = PCI_SUBCLASS_VGA_COMPATIBLE_CONTROLLER;

const static uint16_t Vga_pci_vendor_id = 0x1234;
const static uint16_t Vga_pci_device_id = 0x1111;

const static unsigned Fb_barn = 0;
const static size_t   Fb_size = 16 * MB;

const static unsigned MMIO_barn = 2;
const static size_t   MMIO_size = 4 * KB;

/* vga register region */
#define PCI_VGA_IOPORT_OFFSET 0x400
#define PCI_VGA_IOPORT_SIZE   (0x3e0 - 0x3c0)

/* bochs vbe register region */
#define PCI_VGA_BOCHS_OFFSET 0x500
#define PCI_VGA_BOCHS_SIZE   (0x0b * 2)

/* qemu extension register region */
#define PCI_VGA_QEXT_OFFSET 0x600
#define PCI_VGA_QEXT_SIZE   (2 * 4)

/* qemu extension registers */
#define PCI_VGA_QEXT_REG_SIZE      (0 * 4)
#define PCI_VGA_QEXT_REG_BYTEORDER (1 * 4)
#define PCI_VGA_QEXT_LITTLE_ENDIAN 0x1e1e1e1e
#define PCI_VGA_QEXT_BIG_ENDIAN    0xbebebebe

#endif // JOS_KERN_STANDARD_VGA_H