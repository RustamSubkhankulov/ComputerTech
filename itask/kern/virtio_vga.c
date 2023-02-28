#include <inc/assert.h>
#include <inc/error.h>
#include <inc/uefi.h>
#include <inc/x86.h>
#include <inc/string.h>

#include <kern/virtio_vga.h>
#include <kern/traceopt.h>

static virtio_vga_dev_t Virtio_vga_dev = { 0 };

static int virtio_vga_device_specific_setup();

void init_virtio_vga(void)
{
    // 0) Find Device over PCI bus and check fields with expected values

    int found = pci_dev_find(VIRTIO_VGA_DEV_TO_PCI_DEV(&Virtio_vga_dev),
                                                        Virtio_vga_pci_class,
                                                        Virtio_vga_pci_subclass,
                                                        Virtio_pci_vendor_id);
    if (found == -1)
    {
        cprintf("virtio-vga: device has not been found on PCI bus. \n");
        goto panic;
    }

    int err = pci_dev_general_read_header(VIRTIO_VGA_DEV_TO_PCI_DEV_GENERAL(&Virtio_vga_dev));
    if (err == -1)
    {
        cprintf("virtio-vga: incorrect bus, dev & func parameters. \n");
        goto panic;
    }

    if (trace_gpu)
        dump_pci_dev_general(VIRTIO_VGA_DEV_TO_PCI_DEV_GENERAL(&Virtio_vga_dev));

    // Note: Drivers MUST match any PCI Revision ID value. 
    //       Drivers MAY match any PCI Subsystem Vendor ID and any PCI Subsystem Device ID value.
    //       Non-transitional devices SHOULD have a PCI Device ID in the range 0x1040 to 0x107f. 
    //       Non-transitional devices SHOULD have a PCI Revision ID of 1 or higher. 
    //       Non-transitional devices SHOULD have a PCI Subsystem Device ID of 0x40 or higher.

    if (Virtio_vga_dev.virtio_dev.pci_dev_general.pci_dev.device_id != Virtio_vga_pci_device_id)
    {
        cprintf("virtio-vga: unexpected device_id, revision_id or subsystem_device_id \n");
        goto panic;
    }
    // assert(false);

    // 1) Reset the device

    err = virtio_dev_reset(VIRTIO_VGA_DEV_TO_VIRTIO_DEV(&Virtio_vga_dev));
    if (err != 0)
    {
        cprintf("virtio-vga: virtio device reset failed \n");
        goto panic;
    }

    // 2) Set ACKNOWLEDGE, DRIVER status bits

    err = virtio_dev_init(VIRTIO_VGA_DEV_TO_VIRTIO_DEV(&Virtio_vga_dev));
    if (err != 0)
    {
        cprintf("virtio-vga: virtio general initializarion failed \n");
        goto panic;
    }

    // 3) Negotiate features

    uint32_t requested_f = 0;
    err = virtio_dev_negf(VIRTIO_VGA_DEV_TO_VIRTIO_DEV(&Virtio_vga_dev), requested_f);
    if (err != 0)
    {
        cprintf("virtio-vga: features negotiating failed \n");
        goto panic;
    }

    // 4) Device specific setup

    err = virtio_vga_device_specific_setup();
    if (err != 0)
    {
        cprintf("virtio-vga: device-specific setup failed \n");
        goto panic;
    }

    // 5) Unmask IRQ and set the DRIVER_OK status bit. At this point the device is “live”.

    err = virtio_dev_fin_init(VIRTIO_VGA_DEV_TO_VIRTIO_DEV(&Virtio_vga_dev));
    if (err != 0)
    {
        cprintf("virtio-vga: final general initialization failed \n");
        goto panic;
    }

    if (trace_gpu)
        cprintf("virtio-vga: successully inializated \n");

    return;

panic:

    virtio_set_dev_status_flag(VIRTIO_VGA_DEV_TO_VIRTIO_DEV(&Virtio_vga_dev), VIRTIO_PCI_STATUS_FAILED);
    panic("virtio-vga: initialization failed \n");

}

static int virtio_vga_device_specific_setup()
{
    return 0;
}