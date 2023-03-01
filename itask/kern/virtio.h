#ifndef JOS_KERN_VIRTIO_H
#define JOS_KERN_VIRTIO_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/pci.h>
#include <kern/pmap.h>

const static uint16_t Virtio_pci_vendor_id = 0x1AF4;

#define PCI_DEVICE_ID_CALC_OFFSET 0x1040

/* Device status flags                       Offs */
#define VIRTIO_PCI_STATUS_ACKNOWLEDGE        0x1   
#define VIRTIO_PCI_STATUS_DRIVER             0x2   
#define VIRTIO_PCI_STATUS_DRIVER_OK          0x4
#define VIRTIO_PCI_STATUS_FEATURES_OK        0x20
#define VIRTIO_PCI_STATUS_DEVICE_NEEDS_RESET 0x40  
#define VIRTIO_PCI_STATUS_FAILED             0x80  // driver has given up on the device 

/* This marks a buffer as continuing via the next field. */ 
#define VIRTQ_DESC_F_NEXT     1 

/* This marks a buffer as device write-only (otherwise device read-only). */ 
#define VIRTQ_DESC_F_WRITE    2 

/* This means the buffer contains a list of buffer descriptors. */ 
#define VIRTQ_DESC_F_INDIRECT 4

typedef struct Vring_desc
{
	uint64_t addr;
	uint32_t len;
	uint16_t flags; // above
	uint16_t next;

} vring_desc_t;

typedef struct Vring_avail
{
    uint16_t flags;
	uint16_t idx;
	volatile uint16_t ring[];
    // uint16_t used_event; // Only if VIRTIO_F_EVENT_IDX

} vring_avail_t;

typedef struct Vring_used_elem
{
	uint32_t index;  // Index of start of used descriptor chain.
	uint32_t length; // Total length of the descriptor chain which was used (written to)

} vring_used_elem_t;

#define VIRTQ_USED_F_NO_NOTIFY  1 

typedef struct Vring_used
{
    uint16_t flags;
	uint16_t idx;
	volatile vring_used_elem_t ring[];
    // uint16_t avail_event; // Only if VIRTIO_F_EVENT_IDX

} vring_used_t;

typedef struct Vring
{
    uint16_t num;
    
    volatile vring_desc_t*  desc;
    volatile vring_avail_t* avail;
    volatile vring_used_t*  used;

} vring_t;

typedef struct Virtqueue
{
    vring_t vring;

    uint16_t free_index;     // index of first free descriptor
    uint16_t num_free;       // count of free descriptors 

    uint16_t last_used;      // last seen used
    uint16_t last_avail;

    uint8_t* copy_buf;
    uint32_t chunk_size;

} virtqueue_t;

#define MAX_VIRTQUEUE_SIZE 32768

typedef struct Virtio_pci_cap
{
    uint8_t cap_vndr; /* Generic PCI field: PCI_CAP_ID_VNDR */ 
    uint8_t cap_next; /* Generic PCI field: next ptr. */ 
    uint8_t cap_len; /* Generic PCI field: capability length */
    uint8_t cfg_type;  /* Identifies the structure. */ 
    uint8_t bar;  /* Where to find it. */ 
    uint8_t id; /* Multiple capabilities of the same type */ 
    uint8_t padding[2]; /* Pad to full dword. */ 
    uint32_t offset;  /* Offset within bar. */ 
    uint32_t length; /* Length of the structure, in bytes. */ 

} virtio_pci_cap_t;

typedef struct Virtio_pci_common_cfg 
{ 
    /* About the whole device. */ 
    uint32_t device_feature_select;     /* read-write */ 
    uint32_t device_feature;            /* read-only for driver */ 
    uint32_t driver_feature_select;     /* read-write */ 
    uint32_t driver_feature;            /* read-write */ 
    uint16_t config_msix_vector;        /* read-write */ 
    uint16_t num_queues;                /* read-only for driver */ 
    uint8_t device_status;               /* read-write */ 
    uint8_t config_generation;           /* read-only for driver */ 

    /* About a specific virtqueue. */ 
    uint16_t queue_select;              /* read-write */ 
    uint16_t queue_size;                /* read-write */ 
    uint16_t queue_msix_vector;         /* read-write */ 
    uint16_t queue_enable;              /* read-write */ 
    uint16_t queue_notify_off;          /* read-only for driver */ 
    uint64_t queue_desc;                /* read-write WARNING: must not be read directly*/ 
    uint64_t queue_driver;              /* read-write WARNING: must not be read directly*/ 
    uint64_t queue_device;              /* read-write WARNING: must not be read directly*/ 
    uint16_t queue_notify_data;         /* read-only for driver */ 
    uint16_t queue_reset;               /* read-write */ 

} virtio_pci_common_cfg_t;

typedef struct Virtio_pci_notify_cap 
{ 
    virtio_pci_cap_t cap; 
    uint32_t notify_off_multiplier; /* Multiplier for queue_notify_off. */ 

} virtio_pci_notify_cap_t;

typedef struct Virtio_pci_cfg_cap 
{ 
    virtio_pci_cap_t cap; 
    uint8_t pci_cfg_data[4]; /* Data for BAR access. */ 

} virtio_pci_cfg_cap_t;

// Virtio_pci_cap cfg_type field values: 

/* Common configuration */ 
#define VIRTIO_PCI_CAP_COMMON_CFG        1 
/* Notifications */ 
#define VIRTIO_PCI_CAP_NOTIFY_CFG        2 
/* ISR Status */ 
#define VIRTIO_PCI_CAP_ISR_CFG           3 
/* Device specific configuration */ 
#define VIRTIO_PCI_CAP_DEVICE_CFG        4 
/* PCI configuration access */ 
#define VIRTIO_PCI_CAP_PCI_CFG           5 
/* Shared memory region */ 
#define VIRTIO_PCI_CAP_SHARED_MEMORY_CFG 8 
/* Vendor-specific data */ 
#define VIRTIO_PCI_CAP_VENDOR_CFG        9

// Types from 1 to 5 are supported by driver
#define VIRTIO_PCI_CAP_MIN 1
#define VIRTIO_PCI_CAP_MAX 5

#define VIRTIO_PCI_CAP_NUM 5

typedef struct Virtio_dev
{
    pci_dev_general_t pci_dev_general;

    virtio_pci_cap_t caps[VIRTIO_PCI_CAP_NUM]; 

    volatile virtio_pci_common_cfg_t* common_cfg;
    volatile virtio_pci_notify_cap_t* notify_cap;
    volatile uint8_t* isr;
    volatile virtio_pci_cfg_cap_t* pci_access;

    unsigned queues_n;
    virtqueue_t* queues;
    
    uint64_t features; // features supported by both device & driver

} virtio_dev_t;

#define BUFFER_INFO_F_WRITE (1 << 0) // Buffer is writable
#define BUFFER_INFO_F_COPY  (1 << 1) // Kernel must copy this buffer

typedef struct Buffer_info
{
    uint64_t addr;
    uint32_t len;
    uint16_t flags;

} buffer_info_t;

/* Reserved Feature Bits */
#define VIRTIO_F_RING_INDIRECT_DESC 28 // Negotiating this feature indicates that the driver can use descriptors with the VIRTQ_DESC_F_INDIRECT flag set
#define VIRTIO_F_EVENT_IDX          29 // This feature enables the used_event and the avail_event fields
#define VIRTIO_F_VERSION_1          32 // This indicates compliance with this specification, giving a simple way to detect legacy devices or drivers.
#define VIRTIO_F_ACCESS_PLATFORM    33 // This feature indicates that the device can be used on a platform where device access to data in memory is limited and/or translated
#define VIRTIO_F_RING_PACKED        34 // This feature indicates support for the packed virtqueue layout 
#define VIRTIO_F_IN_ORDER           35 // This feature indicates that all buffers are used by the device in the same order in which they have been made available.
#define VIRTIO_F_ORDER_PLATFORM     36 // This feature indicates that memory accesses by the driver and the device are ordered in a way described by the platform.
#define VIRTIO_F_SR_IOV             37 // This feature indicates that the device supports Single Root I/O Virtualization
#define VIRTIO_F_NOTIFICATION_DATA  38 // This feature indicates that the driver passes extra data (besides identifying the virtqueue) in its device notifications. 
#define VIRTIO_F_NOTIF_CONFIG_DATA  39 // This feature indicates that the driver uses the data provided by the device as a virtqueue identifier in available buffer notifications.
#define VIRTIO_F_RING_RESET         40 // This feature indicates that the driver can reset a queue individually

int virtio_dev_init(virtio_dev_t* virtio_dev);
int virtio_dev_reset(virtio_dev_t* virtio_dev);
int virtio_dev_negf(virtio_dev_t* virtio_dev, uint64_t requested);  
int virtio_dev_fin_init(virtio_dev_t* virtio_dev);  

uint64_t virtio_dev_get_queue_desc(virtio_dev_t* virtio_dev);
uint64_t virtio_dev_get_queue_driver(virtio_dev_t* virtio_dev);
uint64_t virtio_dev_get_queue_device(virtio_dev_t* virtio_dev);

void* get_addr_by_capability(virtio_dev_t* virtio_dev, const virtio_pci_cap_t* cap);

int virtio_setup_virtqueues(virtio_dev_t* virtio_dev, size_t chunk_size);
int virtio_setup_virtqueue(virtqueue_t* virtqueue, uint16_t size, size_t chunk_size);
int virtio_setup_vring(vring_t* vring, uint16_t size);

void dump_virtqueue(const virtqueue_t* virtqueue);

int virtio_snd_buffers(virtio_dev_t* virtio_dev, unsigned qind, const buffer_info_t* buffer_info, unsigned buffers_num);

int virtio_send_avail_notif(virtio_dev_t* virtio_dev, uint16_t qind);

#define ALIGN(x, qalign) (((x) + (qalign - 1)) & (~(qalign - 1))) 
#define QALIGN PAGE_SIZE

static inline void virtio_set_dev_status_flag(const virtio_dev_t* virtio_dev, uint8_t flag)
{
    assert(virtio_dev != NULL);
    virtio_dev->common_cfg->device_status |= flag;
    return;
}

static inline bool virtio_check_dev_status_flag(const virtio_dev_t* virtio_dev, uint8_t flag)
{
    assert(virtio_dev != NULL);
    return (virtio_dev->common_cfg->device_status & flag);
}

static inline size_t vring_calc_size(uint16_t size)
{
    return ALIGN(sizeof(vring_desc_t) * size + sizeof(uint16_t) * (2 + size), QALIGN)
         + ALIGN(sizeof(uint16_t) * 2 + sizeof(vring_used_elem_t) * size, QALIGN);
}

static inline void virtq_used_notif_disable(virtqueue_t* virtqueue)
{
    assert(virtqueue);
    virtqueue->vring.avail->flags = 1; // spec: If flags is 1, the device SHOULD NOT send a notification
}

static inline void virtq_used_notif_enable(virtqueue_t* virtqueue)
{
    assert(virtqueue);
    virtqueue->vring.avail->flags = 0; // spec: If flags is 0, the device MUST send a notification
}

// 1 - should not send; 0 - send avail notification
static inline bool virtq_avail_notif_suppressed_check(const virtqueue_t* virtqueue)
{
    assert(virtqueue);
    return virtqueue->vring.used->flags;
}

#endif /* !JOS_KERN_VIRTIO_H */
