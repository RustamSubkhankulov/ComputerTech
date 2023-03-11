#ifndef JOS_KERN_VGA_H
#define JOS_KERN_VGA_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/assert.h>
#include <inc/x86.h>

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
const static size_t Fb_size = 16 * MB;

const static unsigned MMIO_barn = 2;
const static size_t MMIO_size = 4 * KB;

typedef struct Vga_dev
{
    pci_dev_general_t pci_dev_general;
    
    uint64_t fb;
    uint64_t mmio;

} vga_dev_t;

void init_vga(void);
void test_vga(void);

#define VGA_TO_PCI_GENERAL(vga_dev) (&((vga_dev)->pci_dev_general))
#define VGA_TO_PCI(vga_dev) (&((vga_dev)->pci_dev_general.pci_dev))

#endif // JOS_KERN_VGA_H