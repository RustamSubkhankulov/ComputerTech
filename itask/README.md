DOOM (1993) JOS-port
===================

Description
-----------
DOOM JOS-port is an invidual task, which is based on course 
"Operating system kernel design", DREC MIPT 2022-2023. 

The goal of this project is to port DOOM to JOS - simple operating system used 
as educating example of OS kernel in MIT and MIPT courses. Some parts of the 
kernel have been already enhanced/reworked previosly by me during the course.  
Althouth, project uses version of OS, that is not fully completed yet.
This is up to me to complete course and add missing functions later.

Plan
----
To port DOOM to JOS, few modules of OS must be added: 
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
As graphics adapter, Virtio GPU device is choosed. To implement it, 
PCI bus support firstly has to be added.
Simple implementation for x86 runs over ports 0xCF8 (PCI_CONFIGURATION_ADDRESS_PORT) 
and 0xCFC (PCI_CONFIGURATION_DATA_PORT).

< More description of this OS module will be written. >

Virtual I/O Device (VIRTIO) GPU device
--------------------------------------
