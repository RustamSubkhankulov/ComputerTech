#ifndef JOS_KERN_VGA_PORTS_H
#define JOS_KERN_VGA_PORTS_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

//-----------------
// VGA's I/O ports
//-----------------

// Port 0x3C0: write both the index and data bytes to the same port. 
// The VGA keeps track of whether the next write is supposed to be the index 
// or the data byte. However, the initial state is unknown. By reading 
// from port 0x3DA it'll go to the index state. To read the contents, 
// feed the index into port 0x3C0, then read the value from 0x3C1 
// (then read 0x3DA as it is not defined whether the VGA expects a data byte or index byte next).

// Bit 0 of MISC_OUTPUT_REG controls the location of several other registers: 
// if cleared, port 0x3D4 is mapped to 0x3B4, and port 0x3DA is mapped to 0x3BA. 

#define ATTR_ADDR_DATA_REG           0x3C0
#define ATTR_DATA_READ_REG           0x3C1
#define INP_STATUS_0_REG_READ        0x3C2
#define MISC_OUTPUT_REG_WRITE        0x3C2
#define SEQ_ADDR_REG                 0x3C4
#define SEQ_DATA_REG                 0x3C5
#define DAC_MASK                     0x3C6 // normally 0xFF
#define DAC_STATE_REG_READ           0x3C7
#define DAC_ADDR_READ_MODE_REG_WRITE 0x3C7
#define DAC_ADDR_WRITE_MODE_REG      0x3C8
#define DAC_DATA_REG                 0x3C9
#define FEAT_CTRL_REG_READ           0x3CA
#define MISC_OUTPUT_REG_READ         0x3CC
#define GRAPH_CTRL_ADDR_REG          0x3CE
#define GRAPH_CTRL_DATA_REG          0x3CF
#define CRTC_CTRL_ADDR_REG           0x3D4
#define CRTC_CTRL_DATA_REG           0x3D5
#define INP_STATUS_1_REG_READ        0x3DA
#define FEAT_CTRL_REG_WRITE          0x3DA

#define CRTC_CTRL_ADDR_REG_ALT       0x3B4
#define CRTC_CTRL_DATA_REG_ALT       0x3B5
#define INP_STATUS_1_REG_READ_ALT    0x3BA
#define FEAT_CTRL_REG_WRITE_ALT      0x3BA

#endif // JOS_KERN_VGA_PORTS_H
