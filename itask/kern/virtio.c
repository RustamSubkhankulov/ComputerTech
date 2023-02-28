#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/virtio.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <kern/picirq.h>

static int virtio_dev_read_pci_cap_list(virtio_dev_t* virtio_dev);
static int virtio_dev_prepare_dev_conf(virtio_dev_t* virtio_dev);
static void* get_addr_by_capability(virtio_dev_t* virtio_dev, const virtio_pci_cap_t* cap);
static void read_virtio_pci_cap(pci_dev_t* pci_dev, virtio_pci_cap_t* virtio_pci_cap, uint8_t offs);

int virtio_dev_init(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

    int err = virtio_dev_read_pci_cap_list(virtio_dev);
    if (err < 0) return err;

    virtio_set_dev_status_flag(virtio_dev, VIRTIO_PCI_STATUS_ACKNOWLEDGE);
    virtio_set_dev_status_flag(virtio_dev, VIRTIO_PCI_STATUS_DRIVER);

    return 0;
}

static void read_virtio_pci_cap(pci_dev_t* pci_dev, virtio_pci_cap_t* virtio_pci_cap, uint8_t offs)
{
    virtio_pci_cap->cap_vndr = pci_config_read8(pci_dev, offs);
    virtio_pci_cap->cap_next = pci_config_read8(pci_dev, offs + 1);
    virtio_pci_cap->cap_len  = pci_config_read8(pci_dev, offs + 2);
    virtio_pci_cap->cfg_type = pci_config_read8(pci_dev, offs + 3);
    virtio_pci_cap->bar      = pci_config_read8(pci_dev, offs + 4);
    
    virtio_pci_cap->offset   = pci_config_read32(pci_dev, offs + 8);
    virtio_pci_cap->length   = pci_config_read32(pci_dev, offs + 12);

    return;
}

static int virtio_dev_read_pci_cap_list(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

    uint16_t status = pci_dev_get_stat_reg(&virtio_dev->pci_dev_general.pci_dev);
    if (!(status & STATUS_REG_CAPABILITIES_LIST))
        panic("virtio: device does implement capabilites list \n");

    uint8_t cap_ptr = virtio_dev->pci_dev_general.capabilites_ptr;
    
    uint8_t cfg_type_offs = offsetof(virtio_pci_cap_t, cfg_type);
    uint8_t cap_next_offs = offsetof(virtio_pci_cap_t, cap_next);

    uint8_t caps_read = 0;

    do 
    {
        uint8_t cfg_type = pci_config_read8(&virtio_dev->pci_dev_general.pci_dev, cap_ptr + cfg_type_offs);

        if ((cfg_type < VIRTIO_PCI_CAP_MIN || cfg_type > VIRTIO_PCI_CAP_MAX)
         || (caps_read & (1 << cfg_type)))
        {
            cap_ptr = pci_config_read8(&virtio_dev->pci_dev_general.pci_dev, cap_ptr + cap_next_offs);
            continue;
        }

        caps_read |= (1 << cfg_type);
        read_virtio_pci_cap(&virtio_dev->pci_dev_general.pci_dev, virtio_dev->caps + cfg_type - 1, cap_ptr);

        if (trace_virtio)
        {
            virtio_pci_cap_t* cur = virtio_dev->caps + cfg_type - 1;
            cprintf("next %u len %u type %u bar %u id %u offset %u length %u \n", cur->cap_next, 
                                                                                  cur->cap_len,
                                                                                  cur->cfg_type,
                                                                                  cur->bar,
                                                                                  cur->id,
                                                                                  cur->offset,
                                                                                  cur->length);
        }

        cap_ptr = pci_config_read8(&virtio_dev->pci_dev_general.pci_dev, cap_ptr + cap_next_offs);

    } while (cap_ptr != 0); // reached end of the list

    if (caps_read != 0b111110)
    {
        cprintf("virtio: necessary capability isn't present in cap list \n");
        return -1;
    }

    return virtio_dev_prepare_dev_conf(virtio_dev);
}  

static void* get_addr_by_capability(virtio_dev_t* virtio_dev, const virtio_pci_cap_t* cap)
{
    assert(virtio_dev != NULL);
    assert(cap != NULL);

    uint32_t* BARs = (uint32_t*) &(virtio_dev->pci_dev_general.BAR);
    uint8_t barn = cap->bar;

    cprintf("CONVERTING barToAdrdr: barn %d BARs[barn] == %u \n", barn, BARs[barn]);

    if ((BARs[barn] & MS_BAR_ALWAYS0) == 0)
    {
        if ((BARs[barn] & MS_BAR_TYPE) == MS_BAR_TYPE_64)
        {
            void* bar_addr = (void*) ((uint64_t) (BARs[barn] & 0xFFFFFFF0) + ((uint64_t) (BARs[barn + 1] & 0xFFFFFFFF) << 32));
            cprintf("64bit memory bar_addr %p \n", bar_addr);

            return (void*) ((char*) bar_addr + cap->offset);
        }
        else if ((BARs[barn] & MS_BAR_TYPE) == MS_BAR_TYPE_32)
        {
            void* bar_addr = (void*) (uint64_t) (BARs[barn] & 0xFFFFFFF0);
            cprintf("32bit memory bar_addr %p \n", bar_addr);

            return (void*) ((char*) bar_addr + cap->offset);
        }
        else 
        {
            cprintf("reserved \n");
            return NULL;
        }
    }
    else 
    {
        cprintf("virtio: I/O Space BAR isn't supported \n");
        return NULL;
    }

    return NULL;
}


static int virtio_dev_prepare_dev_conf(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

    // TODO map

    virtio_dev->common_cfg = (virtio_pci_common_cfg_t*) get_addr_by_capability(virtio_dev, 
                                                                               virtio_dev->caps + VIRTIO_PCI_CAP_COMMON_CFG - 1);
    if (virtio_dev->common_cfg == NULL) return -1;
    if (trace_virtio)
        cprintf("virtio_dev->common_cfg %p \n", (void*) virtio_dev->common_cfg);

    virtio_dev->notify_cap = (virtio_pci_notify_cap_t*) get_addr_by_capability(virtio_dev, 
                                                                               virtio_dev->caps + VIRTIO_PCI_CAP_NOTIFY_CFG - 1);
    if (virtio_dev->notify_cap == NULL) return -1;
    if (trace_virtio)
        cprintf("virtio_dev->notify_cap %p \n", (void*) virtio_dev->notify_cap);

    virtio_dev->isr = (uint8_t*) get_addr_by_capability(virtio_dev, 
                                                        virtio_dev->caps + VIRTIO_PCI_CAP_ISR_CFG - 1);
    if (virtio_dev->isr == NULL) return -1;
    if (trace_virtio)
        cprintf("virtio_dev->isr %p \n", (void*) virtio_dev->isr);

    virtio_dev->device_spec = (uint8_t*) get_addr_by_capability(virtio_dev, 
                                                                virtio_dev->caps + VIRTIO_PCI_CAP_DEVICE_CFG - 1);
    if (virtio_dev->device_spec == NULL) return -1;
    if (trace_virtio)
        cprintf("virtio_dev->device_spec %p \n", (void*) virtio_dev->device_spec);

    virtio_dev->pci_access = (virtio_pci_cfg_cap_t*) get_addr_by_capability(virtio_dev, 
                                                                            virtio_dev->caps + VIRTIO_PCI_CAP_PCI_CFG - 1);
    if (virtio_dev->pci_access == NULL) return -1;
    if (trace_virtio)
        cprintf("virtio_dev->pci_access %p \n", (void*) virtio_dev->pci_access);

    return 0;
}


int virtio_dev_reset(virtio_dev_t* virtio_dev)
{
    return 0;
}

int virtio_dev_negf(virtio_dev_t* virtio_dev, uint32_t requested_f)
{
    return 0;
}

int virtio_dev_fin_init(virtio_dev_t* virtio_dev)
{
    return 0;
}

void virtio_set_dev_status_flag(const virtio_dev_t* virtio_dev, uint8_t flag)
{
    assert(virtio_dev != NULL);

    // TODO

    return;
}

bool virtio_check_dev_status_flag(const virtio_dev_t* virtio_dev, uint8_t flag)
{
    return false;
}

int virtio_setup_virtqueue(virtqueue_t* virtqueue, uint16_t size, size_t chunk_size)
{
    return 0;
}

int virtio_setup_vring(vring_t* vring, uint16_t size)
{
    return 0;
}

void dump_virtqueue(const virtqueue_t* virtqueue)
{
    return;
}

