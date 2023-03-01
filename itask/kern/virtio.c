#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/virtio.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <kern/picirq.h>

extern void
map_addr_early_boot(uintptr_t va, uintptr_t pa, size_t sz);

static void* vring_alloc_mem(size_t mem_size);
static uint16_t virtio_descr_add_buffers(virtqueue_t* virtqueue, const buffer_info_t* buffer_info, 
                                                                             unsigned buffers_num);
static int virtqueue_allocate_copy_buf(virtqueue_t* virtqueue, size_t chunk_size);
static int virtio_dev_read_pci_cap_list(virtio_dev_t* virtio_dev);
static int virtio_dev_map_caps(virtio_dev_t* virtio_dev);
static void read_virtio_pci_cap(pci_dev_t* pci_dev, virtio_pci_cap_t* virtio_pci_cap, uint8_t offs);
static int virtio_alloc_virtqueues(virtio_dev_t* virtio_dev);

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

    return virtio_dev_map_caps(virtio_dev);
}  

void* get_addr_by_capability(virtio_dev_t* virtio_dev, const virtio_pci_cap_t* cap)
{
    assert(virtio_dev != NULL);
    assert(cap != NULL);

    uint32_t* BARs = (uint32_t*) &(virtio_dev->pci_dev_general.BAR);
    uint8_t barn = cap->bar;

    void* res = NULL;

    cprintf("CONVERTING barToAdrdr: barn %d BARs[barn] == %u \n", barn, BARs[barn]);

    if ((BARs[barn] & MS_BAR_ALWAYS0) == 0)
    {
        if ((BARs[barn] & MS_BAR_TYPE) == MS_BAR_TYPE_64)
        {
            void* bar_addr = (void*) ((uint64_t) (BARs[barn] & 0xFFFFFFF0) + ((uint64_t) (BARs[barn + 1] & 0xFFFFFFFF) << 32));
            cprintf("64bit memory bar_addr %p \n", bar_addr);

            res = (void*) ((char*) bar_addr + cap->offset);
        }
        else if ((BARs[barn] & MS_BAR_TYPE) == MS_BAR_TYPE_32)
        {
            void* bar_addr = (void*) (uint64_t) (BARs[barn] & 0xFFFFFFF0);
            cprintf("32bit memory bar_addr %p \n", bar_addr);

            res = (void*) ((char*) bar_addr + cap->offset);
        }
        else 
            cprintf("reserved \n");
    }
    else 
        cprintf("virtio: I/O Space BAR isn't supported \n");

    if (res != NULL)
        map_addr_early_boot((uintptr_t) res, (uintptr_t) res, cap->length);

    return res;
}

static int virtio_dev_map_caps(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

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

    cprintf("notify_cap: bar %d offset 0x%u mul %d", virtio_dev->notify_cap->cap.bar, virtio_dev->notify_cap->cap.offset, virtio_dev->notify_cap->notify_off_multiplier);

    virtio_dev->isr = (uint8_t*) get_addr_by_capability(virtio_dev, 
                                                        virtio_dev->caps + VIRTIO_PCI_CAP_ISR_CFG - 1);
    if (virtio_dev->isr == NULL) return -1;
    if (trace_virtio)
        cprintf("virtio_dev->isr %p \n", (void*) virtio_dev->isr);

    virtio_dev->pci_access = (virtio_pci_cfg_cap_t*) get_addr_by_capability(virtio_dev, 
                                                                            virtio_dev->caps + VIRTIO_PCI_CAP_PCI_CFG - 1);
    if (virtio_dev->pci_access == NULL) return -1;
    if (trace_virtio)
        cprintf("virtio_dev->pci_access %p \n", (void*) virtio_dev->pci_access);

    return 0;
}

int virtio_dev_reset(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

    virtio_dev->common_cfg->device_status = 0x0;

    while (virtio_dev->common_cfg->device_status != 0x0)
        continue;

    if (trace_virtio)
        cprintf("virtio: device has been reset. \n");

    return 0;
}

int virtio_dev_negf(virtio_dev_t* virtio_dev, uint64_t requested)
{
    assert(virtio_dev != NULL);

    if (trace_virtio)
        cprintf("virtio: performing features negotiating \n");

    uint32_t req_low  = (uint32_t) (requested & 0xFFFFFFFF); 
    uint32_t req_high = (uint32_t) ((requested >> 32) & 0xFFFFFFFF); 

    virtio_dev->common_cfg->device_feature_select = 0x0; // lower 32 bits
    uint32_t sup_low = virtio_dev->common_cfg->device_feature;

    virtio_dev->common_cfg->device_feature_select = 0x1; // higher 32 bits
    uint32_t sup_high = virtio_dev->common_cfg->device_feature;

    if (trace_virtio)
    {
        cprintf("Guest features (requested): 0x%lx \n", requested);
        cprintf("LOW:  guest 0x%x device 0x%x AND: 0x%x \n", req_low, sup_low, req_low & sup_low);
        cprintf("HIGH: guest 0x%x device 0x%x AND: 0x%x \n", req_high, sup_high, req_high & sup_high);
    }

    if ((req_low & sup_low) != req_low || (req_high & sup_high) != req_high)
    {
        cprintf("virtio: device does not support requested features. \n");
        return -1;
    }

    virtio_dev->common_cfg->driver_feature_select = 0x0;
    virtio_dev->common_cfg->driver_feature = req_low;

    virtio_dev->common_cfg->driver_feature_select = 0x1;
    virtio_dev->common_cfg->driver_feature = req_high;

    virtio_set_dev_status_flag(virtio_dev, VIRTIO_PCI_STATUS_FEATURES_OK);
    if (virtio_check_dev_status_flag(virtio_dev, VIRTIO_PCI_STATUS_FEATURES_OK) != true)
    {
        cprintf("virtio: device does not support our subset of features and the device is unusable \n");
        return -1;
    }

    return 0;
}

int virtio_dev_fin_init(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

    if (virtio_dev->pci_dev_general.interrupt_line != IRQ_VIRTIO)
    {
        cprintf("virtio: device supported only on IRQ 11 \n");

        virtio_set_dev_status_flag(virtio_dev, VIRTIO_PCI_STATUS_FAILED);
        return -1;
    }

    pic_irq_unmask(IRQ_VIRTIO);

    if (trace_virtio)
        cprintf("IRQ_VIRTIO (11) unmasked. \n");

    if (trace_virtio)
        cprintf("virtio: device final initialization completed. Sending DRIVER_OK to device. \n");

    virtio_set_dev_status_flag(virtio_dev, VIRTIO_PCI_STATUS_DRIVER_OK); 

    return 0;
}

static int virtio_alloc_virtqueues(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

    if (trace_virtio)
        cprintf("virtio: allocating memory for virtqueues structures \n");

    virtio_dev->queues_n = virtio_dev->common_cfg->num_queues;
    size_t size = virtio_dev->queues_n * sizeof(virtqueue_t);

    if (trace_virtio)
        cprintf("virtio: num of queues is %d \n", virtio_dev->queues_n);

    int class = 0;
    for (; class < MAX_CLASS; class++)
    {
        if (size < CLASS_SIZE(class))
            break;
    }

    struct Page* allocated = alloc_page(class, 0);
    if (allocated == NULL)
        return -1;

    page_ref(allocated);
    void* memory = (void*) ((uint64_t) page2pa(allocated));

    memset(memory, 0, size);
    virtio_dev->queues = (virtqueue_t*) memory;

    if (trace_virtio)
        cprintf("virtio: allocated memory for virtqueues structures \n");
    return 0;
}

int virtio_setup_virtqueues(virtio_dev_t* virtio_dev, size_t chunk_size) // TODOITASK chunk_size
{
    assert(virtio_dev != NULL);

    if (trace_virtio)
        cprintf("virtio: setting up virtqueues \n");

    int err = virtio_alloc_virtqueues(virtio_dev);
    if (err < 0)
    {
        cprintf("virtio: failed to allocate memory for virtqueues structures \n");
        return err;
    }
    
    for (unsigned qno = 0; qno < virtio_dev->queues_n; qno++)
    {
        virtio_dev->common_cfg->queue_select = qno;
        uint16_t qsize = virtio_dev->common_cfg->queue_size;

        if (qsize == 0)
        {
            cprintf("virtio: queue %d does not exist \n", qno);
            return -1;
        }

        if (trace_virtio)
            cprintf("virtio: queue#%d size 0x%x \n", qno, qsize);

        err = virtio_setup_virtqueue(virtio_dev->queues + qno, qsize, chunk_size);
        if (err < 0)
        {
            cprintf("virtio: failed to setup queue#%d \n", qno);
            return err;
        }

        virtio_dev->common_cfg->queue_select = qno;

        virtio_dev->common_cfg->queue_desc   = (uint64_t) virtio_dev->queues[qno].vring.desc; 
        virtio_dev->common_cfg->queue_driver = (uint64_t) virtio_dev->queues[qno].vring.avail;
        virtio_dev->common_cfg->queue_device = (uint64_t) virtio_dev->queues[qno].vring.used;        
    
        if (trace_virtio)
            cprintf("virtio: queue#%d : desc 0x%lx driver(avail) 0x%lx device(used) 0x%lx \n", qno, (uint64_t) virtio_dev->queues[qno].vring.desc,
                                                                                                    (uint64_t) virtio_dev->queues[qno].vring.avail,
                                                                                                    (uint64_t) virtio_dev->queues[qno].vring.used);
    }

    if (trace_virtio)
        cprintf("virtio: virtqueues were successfully set up \n");
    return 0;
}

int virtio_setup_virtqueue(virtqueue_t* virtqueue, uint16_t size, size_t chunk_size)
{
    assert(virtqueue != 0);

    int err = virtio_setup_vring(&(virtqueue->vring), size);
    if (err != 0) return err;

    virtqueue->free_index = 0;
    virtqueue->last_used  = 0;
    virtqueue->last_avail = 0;
    virtqueue->num_free   = size;

    err = virtqueue_allocate_copy_buf(virtqueue, chunk_size);
    if (err != 0) return err;

    return 0;
}

int virtio_setup_vring(vring_t* vring, uint16_t size)
{
    assert(vring != 0);

    size_t mem_size = vring_calc_size(size);

    void* memory = vring_alloc_mem(mem_size);
    if (memory == NULL)
        return -1;

    if ((uint64_t) memory > 4 * GB)
    {
        cprintf("virtio: vring setup failed, physical addresses higher than 4 are not supported. ");
        return -1;
    }

    memset(memory, 0x0, mem_size);
    vring->num = size;

    vring->desc  = (vring_desc_t*) memory;
    vring->avail = (vring_avail_t*) ((unsigned char*) memory + sizeof(vring_desc_t) * size);
    vring->used  = (vring_used_t*) &((unsigned char*) memory)[ALIGN(sizeof(vring_desc_t) * size + sizeof(uint16_t) * (2 + size), QALIGN)];

    return 0;
}

void dump_virtqueue(const virtqueue_t* virtqueue)
{
    assert(virtqueue != 0);

    cprintf("FREE_INDEX: %d LAST_USED: %d LAST_AVAIL: %d \n", virtqueue->free_index,
                                                              virtqueue->last_used,
                                                              virtqueue->last_avail);
    cprintf("NUM_FREE: %d \n", virtqueue->num_free);
    cprintf("VRING.NUM %d \n", virtqueue->vring.num);

    cprintf("VRING.DESC: \n");
    for (unsigned iter = 0; iter < virtqueue->vring.num; iter++)
    {
        cprintf("DESC[%03d]: addr: 0x%lx len: %d flags: 0x%x next: %d | ", iter, virtqueue->vring.desc[iter].addr,
                                                                                 virtqueue->vring.desc[iter].len,
                                                                                 virtqueue->vring.desc[iter].flags,
                                                                                 virtqueue->vring.desc[iter].next);
        
        for (unsigned ind = 0; ind < virtqueue->vring.desc[iter].len; ind++)
        {
            cprintf("[%c]", *((char*) (virtqueue->vring.desc[iter].addr) + ind));
        }

        if ((iter % 2) == 0)
            cprintf("\n");
    }

    cprintf("VRING.AVAIL: flags: 0x%x idx: %d \n", virtqueue->vring.avail->flags, virtqueue->vring.avail->idx);
    for (unsigned iter = 0; iter < virtqueue->vring.num; iter++)
    {
        cprintf("RING[%03d]: 0x%x | ", iter, virtqueue->vring.avail->ring[iter]);
        if ((iter % 4) == 0)
            cprintf("\n");
    }

    cprintf("VRING.USED: flags: 0x%x idx: %d \n", virtqueue->vring.used->flags, virtqueue->vring.used->idx);
    for (unsigned iter = 0; iter < virtqueue->vring.num; iter++)
    {
        cprintf("RING[%03d]: index: %d lenght: %d | ", iter, virtqueue->vring.used->ring[iter].index, 
                                                             virtqueue->vring.used->ring[iter].length);
        if ((iter % 3) == 0)
            cprintf("\n");
    }

    return;
}

int virtio_send_avail_notif(virtio_dev_t* virtio_dev, uint16_t qind)
{
    assert(virtio_dev != NULL);
    assert(qind < virtio_dev->queues_n);

    // TODOITASK 

    static uint64_t bar_addr = 0;
    uint16_t barn = 0;

    if (bar_addr == 0)
    {
        barn = virtio_dev->notify_cap->cap.bar;
        uint32_t* BARs = (uint32_t*) &(virtio_dev->pci_dev_general.BAR);

        if ((BARs[barn] & MS_BAR_ALWAYS0) == 0)
        {
            if ((BARs[barn] & MS_BAR_TYPE) == MS_BAR_TYPE_64)
            {
                bar_addr = ((uint64_t) (BARs[barn] & 0xFFFFFFF0) + ((uint64_t) (BARs[barn + 1] & 0xFFFFFFFF) << 32));
            }
            else if ((BARs[barn] & MS_BAR_TYPE) == MS_BAR_TYPE_32)
            {
                bar_addr = (uint64_t) (BARs[barn] & 0xFFFFFFF0);
            }
            else 
                return -1;
        }
        else
        {
            cprintf("virtio: I/O Space BAR isn't supported \n");
            return -1;
        }
    }

    virtio_dev->common_cfg->queue_select = qind;
    uint16_t off = virtio_dev->common_cfg->queue_notify_off;

    if (trace_virtio)
        cprintf("sending avail notif: barn %d bar_addr 0x%lx mul %u off 0x%x \n", barn, bar_addr, 
                                              virtio_dev->notify_cap->notify_off_multiplier, off);

    uint16_t* addr = (uint16_t*) ((char*) bar_addr + off * virtio_dev->notify_cap->notify_off_multiplier);
    *addr =  qind;

    return 0;
}

int virtio_snd_buffers(virtio_dev_t* virtio_dev, unsigned qind, const buffer_info_t* buffer_info, unsigned buffers_num)
{
    assert(virtio_dev != 0);
    assert(buffer_info != 0);
    assert(qind < virtio_dev->queues_n);
    assert(buffers_num < virtio_dev->queues[qind].vring.num);

    if (buffers_num == 0)
        return 0;

    virtqueue_t* virtqueue = virtio_dev->queues + qind;
    if (virtqueue->num_free < buffers_num)
    {
        cprintf("virtio: Not enough space in descr table. virtio_snd_buffers() failed. \n");
        return -1;
    }

    uint16_t chain_head = virtio_descr_add_buffers(virtqueue, buffer_info, buffers_num);
    
    uint16_t avail_ind = virtqueue->vring.avail->idx % virtqueue->vring.num;
    virtqueue->vring.avail->ring[avail_ind] = chain_head;
    virtqueue->last_avail = avail_ind;

    mfence();
    virtqueue->vring.avail->idx += 1;
    mfence();

    if (!virtq_avail_notif_suppressed_check(virtqueue))
    {
        if (trace_virtio)
            cprintf("virtio: Avail notifications are not suppressed. Sending notification \n");

        int err = virtio_send_avail_notif(virtio_dev, (uint16_t) qind);
        if (err < 0)
        {
            cprintf("virtio: failed to send avail notif \n");
            return err;
        }
    }

    return 0;
}

static uint16_t virtio_descr_add_buffers(virtqueue_t* virtqueue, const buffer_info_t* buffer_info, 
                                                                             unsigned buffers_num)
{
    assert(buffer_info != 0);
    assert(virtqueue != 0);

    uint16_t descr_free = virtqueue->free_index;
    uint16_t ret_value  = descr_free; 

    uint16_t next_descr_free = 0;

    for (unsigned iter = 0; iter < buffers_num; iter++)
    {
        next_descr_free = (descr_free + 1) % virtqueue->vring.num;
        const buffer_info_t* cur_buffer = buffer_info + iter;

        virtqueue->vring.desc[descr_free].len  = cur_buffer->len;

        if (cur_buffer->flags & BUFFER_INFO_F_COPY)
        {
            virtqueue->vring.desc[descr_free].addr = (uint64_t) (virtqueue->copy_buf 
                                                  + descr_free * virtqueue->chunk_size);

            // clear memory before using buffer
            memset(virtqueue->copy_buf + descr_free * virtqueue->chunk_size, 0, virtqueue->chunk_size);

            memcpy(virtqueue->copy_buf + descr_free * virtqueue->chunk_size, (void*) cur_buffer->addr, 
                                                                                     cur_buffer->len);
        }
        else
            virtqueue->vring.desc[descr_free].addr = cur_buffer->addr;

        if (iter != buffers_num - 1)
            virtqueue->vring.desc[descr_free].flags |= VIRTQ_DESC_F_NEXT;

        if (cur_buffer->flags & BUFFER_INFO_F_WRITE)
            virtqueue->vring.desc[descr_free].flags |= VIRTQ_DESC_F_WRITE;

        uint16_t buffer_next = (iter == buffers_num - 1)? 0: next_descr_free;
        virtqueue->vring.desc[descr_free].next = buffer_next;

        descr_free = next_descr_free;
    }

    virtqueue->free_index = descr_free;
    virtqueue->num_free -= buffers_num;

    return ret_value;
}

static void* vring_alloc_mem(size_t mem_size)
{
    int class = 0; 

    for (; class < MAX_CLASS; class++)
    {
        if (mem_size <= CLASS_SIZE(class))
            break;
    }

    struct Page* allocated_page = alloc_page(class, 0);
    if (allocated_page == NULL)
        return NULL;

    page_ref(allocated_page);

    return (void*) ((uint64_t) page2pa(allocated_page));
}

static int virtqueue_allocate_copy_buf(virtqueue_t* virtqueue, size_t chunk_size)
{
    assert(virtqueue != 0);

    virtqueue->chunk_size = chunk_size;
    size_t copy_buf_size  = chunk_size * virtqueue->vring.num;

    int class = 0;

    for (; class < MAX_CLASS; class++)
    {
        if (copy_buf_size < CLASS_SIZE(class))
            break;
    }

    struct Page* allocated = alloc_page(class, 0);
    if (allocated == NULL) return -1;
    
    page_ref(allocated);
    virtqueue->copy_buf = (uint8_t*) page2pa(allocated);
    memset(virtqueue->copy_buf, 0, copy_buf_size);

    return 0;
}

uint64_t virtio_dev_get_queue_desc(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

    uint64_t before = 0;
    uint64_t after  = 0;

    do
    {
        before = virtio_dev->common_cfg->queue_desc;
        after  = virtio_dev->common_cfg->queue_desc;

    } while (before != after);

    return after;
}

uint64_t virtio_dev_get_queue_driver(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

    uint64_t before = 0;
    uint64_t after  = 0;

    do
    {
        before = virtio_dev->common_cfg->queue_driver;
        after  = virtio_dev->common_cfg->queue_driver;

    } while (before != after);

    return after;
}

uint64_t virtio_dev_get_queue_device(virtio_dev_t* virtio_dev)
{
    assert(virtio_dev != NULL);

    uint64_t before = 0;
    uint64_t after  = 0;

    do
    {
        before = virtio_dev->common_cfg->queue_device;
        after  = virtio_dev->common_cfg->queue_device;

    } while (before != after);

    return after;}


