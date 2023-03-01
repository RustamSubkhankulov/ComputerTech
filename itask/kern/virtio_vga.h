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

#define VIRTIO_GPU_F_VIRGL         0 // virgl 3D mode is supported.
#define VIRTIO_GPU_F_EDID          1 // EDID is supported.
#define VIRTIO_GPU_F_RESOURCE_UUID 2 // assigning resources UUIDs for export to other virtio devices is supported.
#define VIRTIO_GPU_F_RESOURCE_BLOB 3 // creating and using size-based blob resources is supported.
#define VIRTIO_GPU_F_CONTEXT_INIT  4 //multiple context types and synchronization timelines supported. Requires VIRTIO_GPU_F_VIRGL.

#define VIRTIO_GPU_EVENT_DISPLAY (1 << 0) 

/* Virtqueues indexes */
#define CONTROLQ 0
#define CURSORQ  1

typedef struct virtio_gpu_config 
{ 
    uint32_t events_read; 
    uint32_t events_clear; 
    uint32_t num_scanouts; 
    uint32_t num_capsets; 

} virtio_gpu_config_t;

typedef struct Virtio_vga_dev
{
    virtio_dev_t virtio_dev;
    virtio_gpu_config_t* gpu_conf;

} virtio_vga_dev_t;

#define VIRTIO_VGA_DEV_TO_VIRTIO_DEV(virtio_vga_dev) &((virtio_vga_dev)->virtio_dev)
#define VIRTIO_VGA_DEV_TO_PCI_DEV_GENERAL(virtio_vga_dev) &((virtio_vga_dev)->virtio_dev.pci_dev_general)
#define VIRTIO_VGA_DEV_TO_PCI_DEV(virtio_vga_dev) &((virtio_vga_dev)->virtio_dev.pci_dev_general.pci_dev)

#endif /* !JOS_KERN_NET_H */