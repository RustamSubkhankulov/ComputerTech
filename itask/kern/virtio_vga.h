#ifndef JOS_KERN_VIRTIO_VHA_H
#define JOS_KERN_VIRTIO_VHA_H
#ifndef JOS_KERNEL 
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <kern/virtio.h>

void init_virtio_vga(void);

enum Pci_display_controller_subclass
{
    PCI_SUBCLASS_VGA_COMPATIBLE_CONTROLLER = 0x0,
    PCI_SUBCLASS_XGA_CONTROLLER            = 0x1,
    PCI_SUBCLASS_3D_CONTROLLER             = 0x2,
    PCI_SUBCLASS_OTHER                     = 0x80
};

const static enum Pci_class 
                  Virtio_vga_pci_class = PCI_CLASS_DISPLAY_CONTROLLER;

const static enum Pci_display_controller_subclass 
                  Virtio_vga_pci_subclass = PCI_SUBCLASS_VGA_COMPATIBLE_CONTROLLER;

// const static enum Pci_display_controller_subclass 
//                   Virtio_vga_pci_subclass = PCI_SUBCLASS_OTHER;

const static uint16_t Virtio_vga_device_id     = 0x10;
const static uint16_t Virtio_vga_pci_device_id = 0x1040 + Virtio_vga_device_id;

typedef struct Virtio_vga_dev
{
    virtio_dev_t virtio_dev;

} virtio_vga_dev_t;

#define VIRTIO_VGA_DEV_TO_VIRTIO_DEV(virtio_vga_dev) &((virtio_vga_dev)->virtio_dev)
#define VIRTIO_VGA_DEV_TO_PCI_DEV_GENERAL(virtio_vga_dev) &((virtio_vga_dev)->virtio_dev.pci_dev_general)
#define VIRTIO_VGA_DEV_TO_PCI_DEV(virtio_vga_dev) &((virtio_vga_dev)->virtio_dev.pci_dev_general.pci_dev)

#endif /* !JOS_KERN_NET_H */