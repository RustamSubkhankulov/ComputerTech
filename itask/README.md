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
- PCI bus support (to find GPU on it)
- implement support for software graphics rendering 
- input device to read commands from player(such as keyboard)
- system timer 

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

PCI-support module of OS consists of [kern/pci.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/pci.c) and [kern/pci.h](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/pci.h). 

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

- Device ID: Identifies the particular device. Where valid IDs are allocated by the vendor.
- Vendor ID: Identifies the manufacturer of the device. Where valid IDs are allocated by PCI-SIG (the list is here) to ensure uniqueness and 0xFFFF is an invalid value that will be returned on read accesses to Configuration Space registers of non-existent devices.
- Status: A register used to record status information for PCI bus related events.
- Command: Provides control over a device's ability to generate and respond to PCI cycles. Where the only functionality guaranteed to be supported by all devices is, when a 0 is written to this register, the device is disconnected from the PCI bus for all accesses except Configuration Space access.
- Class Code: A read-only register that specifies the type of function the device performs.
- Subclass: A read-only register that specifies the specific function the device performs.
- Prog IF(Programming Interface Byte): A read-only register that specifies a register-level programming interface the device has, if it has any at all.
- Revision ID: Specifies a revision identifier for a particular device. Where valid IDs are allocated by the vendor.
- BIST: Represents that status and allows control of a devices BIST (built-in self test).
- Header Type: Identifies the layout of the rest of the header beginning at byte 0x10 of the header. If bit 7 of this register is set, the device has multiple functions; otherwise, it is a single function device. Types:
0x0: a general device
0x1: a PCI-to-PCI bridge
0x2: a PCI-to-CardBus bridge.
- Latency Timer: Specifies the latency timer in units of PCI bus clocks.
- Cache Line Size: Specifies the system cache line size in 32-bit units. A device can limit the number of cacheline sizes it can support, if a unsupported value is written to this field, the device will behave as if a value of 0 was written.

More information about particular types' headers you can find at [OSDev](https://wiki.osdev.org/PCI)

Standard VGA using VBE interface 
---------------------------------

### vga std

QEMU option: <code>-vga std</code> or <code>-device VGA</code>

PCI ID: 1234:1111

This is the default display device (on x86). It provides full VGA compatibility and support for a simple linear framebuffer (using the bochs dispi interface). It is the best choice compatibility wise, pretty much any guest should be able to bring up a working display on this device. Supports page-flipping.

The device has 16 MB of video memory by default. This can be changed using the vgamem_mb property, -device VGA,vgamem_mb=32 for example will double the amount of video memory.

Used PCI memory / IO-space regions:
- PCI Region 0:
   Framebuffer memory.

### Bochs VBE Extensions

The Bochs VGA BIOS supports, to an extent, the VBE specification. Since Bochs only emulates a VGA card down to the hardware level (and a Cirrus graphics card if you enable it, but that is not tied in with the Bochs VBE extensions), it emulates very simple graphics hardware that the VBE BIOS can drive. The advantage of this is that if you are running your OS in Bochs (or QEMU, which uses the Bochs VGA BIOS, or even VirtualBox), you can use this emulated hardware to directly set video modes without using VBE (which would require real mode or v86).

The Bochs emulated graphics hardware (henceforth called BGA for Bochs Graphics Adaptor) is accessed via two 16-bit IO-ports. The first one is an index port, the second one a data port (comparable to how the VGA handles its sets of registers). Via these ports it is possible to enable or disable the VBE extensions, change the screen resolution and bit depth, and manage a larger virtual screen. There are six versions of the BGA (0xB0C0 through 0xB0C5), but if you use the latest version of Bochs you only need to concern yourself with the latest one (0xB0C5). QEMU (with the -std-vga command line argument) also uses the latest version.

To write an index/data pair to one of the BGA registers, first write its index value to the 16-bit IO-port VBE_DISPI_IOPORT_INDEX (0x01CE), followed by writing the data value to the 16-bit IO-port VBE_DISPI_IOPORT_DATA (0x01CF). The BGA supports 10 different index values (0 through 9):

- VBE_DISPI_INDEX_ID (0)
- VBE_DISPI_INDEX_XRES (1)
- VBE_DISPI_INDEX_YRES (2)
- VBE_DISPI_INDEX_BPP (3)
- VBE_DISPI_INDEX_ENABLE (4)
- VBE_DISPI_INDEX_BANK (5)
- VBE_DISPI_INDEX_VIRT_WIDTH (6)
- VBE_DISPI_INDEX_VIRT_HEIGHT (7)
- VBE_DISPI_INDEX_X_OFFSET (8)
- VBE_DISPI_INDEX_Y_OFFSET (9)

More information about BGA you can find at [OSDev](https://wiki.osdev.org/Bochs_VBE_Extensions)

In addition, a simple graphics lib for drawing simple geometrical objects implemented in [kern/graphics.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/graphics.c), [kern/graphics.h](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/graphics.h)

See [kern/gpu.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/gpu.c), [kern/gpu.h](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/gpu.h), [kern/vbe.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/vbe.c), [kern/vbe.h](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/vbe.h), [kern/graphics.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/graphics.c), [kern/graphics.h](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/graphics.h)

Serial Port
------------

Serial ports are a legacy communications port common on IBM-PC compatible computers. Use of serial ports for connecting peripherals has largely been deprecated in favor of USB and other modern peripheral interfaces, however it is still commonly used in certain industries for interfacing with industrial hardware such as CNC machines or commercial devices such as POS terminals. Historically it was common for many dial-up modems to be connected via a computer's serial port, and the design of the underlying UART hardware itself reflects this.

Serial ports are typically controlled by UART hardware. This is the hardware chip responsible for encoding and decoding the data sent over the serial interface. Modern serial ports typically implement the RS-232 standard, and can use a variety of different connector interfaces. The DE-9 interface is the one most commonly used connector for serial ports in modern systems.

Serial ports are of particular interest to operating-system developers since they are much easier to implement drivers for than USB, and are still commonly found in many x86 systems. It is common for operating-system developers to use a system's serial ports for debugging purposes, since they do not require sophisticated hardware setups and are useful for transmitting information in the early stages of an operating-system's initialization. Many emulators such as QEMU and Bochs allow the redirection of serial output to either stdio or a file on the host computer.

To read pressed keys from keyboard from host system, QEMU option <code>-serial mon:stdio</code> used. In this mode, the virtual serial port and QEMU monitor are multiplexed onto stdio.

Host system receives data using COM1 serial port. 

More information about Serial Ports you can find at [OSDev](https://wiki.osdev.org/Serial_Ports)

See [kern/console.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/console.c)

High Precision Event Timer
---------------------------

To implement main game loop of Tetris at particular framerate, timer support is necessary. 

To measure precise time intervals, tsc is used in conjunction with the HPET timer. The timer measures the processor frequency, and the rtsc command reads the number of processor cycles.

See [kern/ktimer.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/ktimer.c), [kern/ktimer.h](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/ktimer.h), [kern/timer.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/timer.c). [kern/timer.h](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/timer.h)

Finally, the game itself
-------------------------

Using all the above modules, Tetris was written.

Build & usage: <code>ITASK=1 make qemu</code>
If there is any compilation errors, use <code>make distclean</code>, <code>make clean</code> before build. If UDK remote repository is updated and any errors occured, LoaderPkg/UDK directiry should be removed with  <code>-rf</code>.

See [kern/tetris_model.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/tetris_model.c), [kern/tetris_model.h](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/tetris_model.h), [kern/tetris_view.c](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/tetris_view.c), [kern/tetris_view.h](https://github.com/RustamSubkhankulov/ComputerTech/blob/main/itask/kern/tetris_view.h)
