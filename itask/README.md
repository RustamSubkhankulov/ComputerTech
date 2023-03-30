TETRIS (1984) JOS-game
===================

<img src="https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/pictures/Tetris_Logo.jpeg" data-canonical-src="https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/pictures/Tetris_Logo.jpeg" width="600" height="400" />

Description
-----------
TETRIS JOS-game is an invidual task, which is based on course 
"Operating system kernel design", DREC MIPT 2022-2023. 

The goal of this project is to implement TETRIS in JOS - simple operating system used 
as educating example of OS kernel in MIT and MIPT courses. Some parts of the 
kernel have been already enhanced/reworked previosly by me during the course.  
Althouth, project uses version of OS, that is not fully completed yet.
This is up to me to complete course and add missing functions later.

Plan
----
To implement TETRIS in JOS, few modules of OS must be added: 
- implement support for software graphics rendering in the JOS operating system.
- implement support for input devices (such as keyboard or gamepad)

Graphics adapter driver
-----------------------
One of the important components of the operating system is support graphic output. 
The interfaces provided by this component are allow you to implement a graphical 
user interface, professional video and graphics software, computer games and
demo scenes. As part of an individual task, it is proposed to implement
support for software graphics rendering in the JOS operating system.

The standard development scheme for any graphics stack is
the definition of a low-level API by the operating system developer, and
its further implementation by the GPU manufacturer or a separate developer
GPU drivers.

PCI bus support
---------------
As graphics adapter, standart vga dirver is choosed. To implement it, 
PCI bus support firstly has to be added.
Simple implementation for x86 runs over ports 0xCF8 (PCI_CONFIGURATION_ADDRESS_PORT) 
and 0xCFC (PCI_CONFIGURATION_DATA_PORT).

PCI-support module of OS consists of kern/pci.c and kern/pci.h. 

Two 32-bit I/O locations are used, the first location (0xCF8) is named CONFIG_ADDRESS, and the second (0xCFC) is called CONFIG_DATA. CONFIG_ADDRESS specifies the configuration address that is required to be accesses, while accesses to CONFIG_DATA will actually generate the configuration access and will transfer the data to or from the CONFIG_DATA register.

The CONFIG_ADDRESS is a 32-bit register with the format shown in following figure. Bit 31 is an enable flag for determining when accesses to CONFIG_DATA should be translated to configuration cycles. Bits 23 through 16 allow the configuration software to choose a specific PCI bus in the system. Bits 15 through 11 select the specific device on the PCI Bus. Bits 10 through 8 choose a specific function in a device (if the device supports multiple functions).

The least significant byte selects the offset into the 256-byte configuration space available through this method. Since all reads and writes must be both 32-bits and aligned to work on all implementations, the two lowest bits of CONFIG_ADDRESS must always be zero, with the remaining six bits allowing you to choose each of the 64 32-bit words. If you don't need all 32 bits, you'll have to perform the unaligned access in software by aligning the address, followed by masking and shifting the answer.

| **Bit 31** | **Bits 30-24** | **Bits 23-16** | **Bits 15-11** | **Bits 10-8**   | **Bits 7-0**    |
|------------|----------------|----------------|----------------|-----------------|-----------------|
| Enable Bit | Reserved       | Bus Number     | Device Number  | Function Number | Register Offset |

The following field descriptions are common to all Header Types:

| **Register** | **Offset** | **Bits 31-24** | **Bits 23-16** | **Bits 15-8** | **Bits 7-0**    |
|--------------|------------|----------------|----------------|---------------|-----------------|
| 0x0          | 0x0        | Device ID      | Device ID      | Vendor ID     | Vendor ID       |
| 0x1          | 0x4        | Status         | Status         | Command       | Command         |
| 0x2          | 0x8        | Class code     | Subclass       | Prog IF       | Revision ID     |
| 0x3          | 0xC        | BIST           | Header type    | Latency Timer | Cache Line Size |

Device ID: Identifies the particular device. Where valid IDs are allocated by the vendor.
Vendor ID: Identifies the manufacturer of the device. Where valid IDs are allocated by PCI-SIG (the list is here) to ensure uniqueness and 0xFFFF is an invalid value that will be returned on read accesses to Configuration Space registers of non-existent devices.
Status: A register used to record status information for PCI bus related events.
Command: Provides control over a device's ability to generate and respond to PCI cycles. Where the only functionality guaranteed to be supported by all devices is, when a 0 is written to this register, the device is disconnected from the PCI bus for all accesses except Configuration Space access.
Class Code: A read-only register that specifies the type of function the device performs.
Subclass: A read-only register that specifies the specific function the device performs.
Prog IF(Programming Interface Byte): A read-only register that specifies a register-level programming interface the device has, if it has any at all.
Revision ID: Specifies a revision identifier for a particular device. Where valid IDs are allocated by the vendor.
BIST: Represents that status and allows control of a devices BIST (built-in self test).
Header Type: Identifies the layout of the rest of the header beginning at byte 0x10 of the header. If bit 7 of this register is set, the device has multiple functions; otherwise, it is a single function device. Types:
0x0: a general device
0x1: a PCI-to-PCI bridge
0x2: a PCI-to-CardBus bridge.
Latency Timer: Specifies the latency timer in units of PCI bus clocks.
Cache Line Size: Specifies the system cache line size in 32-bit units. A device can limit the number of cacheline sizes it can support, if a unsupported value is written to this field, the device will behave as if a value of 0 was written.

More information about particular types' headers you can find at [OSDev](https://wiki.osdev.org/PCI)

Standard VGA  
--------------------------------------

Initially, I planned to implement virtio-vga device support as graphics adapter, but due to the various bugs and device behaviour, that was not stated in documentation, standard vga was chosen. (But you can still find basic initialization sequence of virtio-vga implemented in /examples folder.)

VGA device is found over PCI. PCI support described in previous part.



< More description of this OS module will be written. >
