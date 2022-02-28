# WARNINGS
Due to compatibility issues regarding the dynamic reconfiguration and the IPs, it is crucial to use the Vivado and Petalinux 2017.4 versions.
Also note that apparently the Zedboard only works if the UART is connected, even if it is not used.

# Introduction

## Motivation
The main idea of this project was to combine IoT test capabilities with dynamic reconfiguration. On the one hand, we were interested in IoT devices and their specific protocols in general. On the other hand, we wanted to experiment with dynamic reconfiguration inside the ZYNQ platform.

## Project idea
![ProjectOverview](ProjectOverview.png)
The basic idea is to use the development board "Zedboard" (based on the ZYNQ platform) as a protocol detector for IoT devices. For proof of concept, we implemented two different protocols, I2C and UART. The protocols are realised directly in hardware in the programming logic (PL) of the board. To bring dynamic reconfiguration in the project, we only use one of the two protocol IP blocks and use dynamic reconfiguration to switch between these FPGA blocks. 

![Procedure](Procedure.png)
To test our design, we implemented two dummy IoT devices on an STM32 Nucleo Board. We try one of our protocols and check if the IoT device answers accordingly. If this is not the case, we switch to the next protocol until we found the right one.

## Overview of programs
* Vivado: programming logic design
* Petalinux: Os-design for Zedboard
* XSDK: C-Code for Zedboard
* Keil uVision: C-Code for Nucleo Board

# Folder Structure
```
.
├── Documentation
├── Keil
├── petalinux
├── README.md
├── srcLinuxApp -> ./Vivado/I2C2/project_1/project_1.sdk/I2C_test3/
└── Vivado
```

```
srcLinuxApp
├── Debug
│   ├── I2C_test3.elf
│   ├── I2C_test3.elf.size
│   ├── makefile
│   ├── objects.mk
│   ├── sources.mk
│   └── src
│       ├── flash.d
│       ├── flash.o
│       ├── helloworld.d
│       ├── helloworld.o
│       └── subdir.mk
├── RemoteSystemsTempFiles
├── SDK.log
└── src
    ├── flash.c
    ├── flash.h
    └── helloworld.c
```

# Vivado 2017.4
As we were using the Zedboard with the Zynq processor from Xilinx, we also used the development environment from Xilinx which is called Vivado. It allows to write Verilog and VHDL and it makes it also possible to structurize the design in a hierarchical way. Furthermore, Xilinx IP cores can be added from a list and connected via a block design. This structure can automatically be wrapped to an HDL code. After that it can run the standard synthesize process.

## Projects
Because we used dynamic reconfiguration for switching the protocols, we had to create two designs which look similar. Both have in common that they contain the Zynq block with its busses and peripheral components including I/O blocks for buttons and LEDs. The only difference are the IP cores which are used for communication.

The first design contains the IP core for the UART communication
![VivadoUart](VivadoUart)
The second one contains the IP core for the I2C communication
![VivadoI2C](VivadoI2C)
Both designs are synthesized independently and switched by the processor at runtime, depending on the application inside.  

Note that the Zynq block is also included in the block design. This is just used for configuration and not for synthesizing, because the processor exists as a hard core and is not realised in the programming logic (PL).
The interrupt signals of both communication blocks are connected to the processing system. They are not directly used by our code but used by the kernel. Not connecting the interrupts results in the kernel not starting properly.  

## IPs
We chose both IP cores from the Xilinx IP library, because these cores can be controlled using the AXI bus, which is used to communicate with the processor and there were drivers provided to call it from Petalinux. 
The IP used for UART communication is the AXI UART Lite. It offers on the one side the AXI bus for processor communication and on the other side the UART lines RX and TX. The baud rate of this IP is configured at design time.
The core for I2C is called AXI IIC Bus Interface and works in a similar way. It is also connected to the AXI bus and offers the two I2C signals SDA (serial data) and SCL (serial clock). It can operate in both I2C modes (standard with 100 kHz and fast with 400 kHz)

The drivers for Petalinux allow to call the UART and I2C interfaces, like any other serial interface in Linux, by writing and reading a file. These filehandlers were used in the c-code to use the interface from our program. To get other information beside the file content (e.g., if bytes are available to read) the Linux command ioctl was used. 

## Usage
To generate a bootable Linux image including the two synthesized bitsreams, you can either use the already generated .hdf files in the repository or generate them out of Vivado. To do so, first the Bitstream Generation in the opened Vivado project has to be triggered. After finishing, the HW-File has to be exported by choosing "File -> Export -> Export Hardware..." and checking the "Include Bitstream" option. 
 
# Petalinux 2017.4
Petalinux is a tool suit created by Xilinx to allow the quick generation of a linux kernel running on an Xilinx FPGA. We use it to quickly generate the OS for the dual ARM A9 core inside the ZYNQ XC7Z020-CLG484-1 on the Zedboard.  

We decided to use petalinux because a operating system was needed to provide multitasking as well as scheduling of different IO operations. It also comes with a lot of prebuild kernel drivers for low level connectivity we needed. For example the uart and I2C drivers where already interrupt driven and ready to use. This allowed us to quickly develop the software on a higher level.  

We use the following commands to extract the vivado generated handoff file which contains the programmable logic (PL) bitstream as well as the ZYNQ processing system (PS) configuration. This is needed to configure the kernel with the correct parameters as well as for the correct device tree generation. After the hdf extraction, some options where enabled in the petalinux configmenu which are checked in, so we don't go into detail.   
The second command actually builds the runnable image. This takes some time, so feel free to take a coffee break. :D  
Finally the packaging will generate a `BOOT.bin` as well as a `image.ub` which then need to be copied to a FAT formatted SD card. To format the SD card, use [this guide](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18842385/How+to+format+SD+card+for+SD+boot).

```
petalinux-config --get-hw-description=../<FOLDER_TO_HDF>
petalinux-build
petalinux-package --boot --format BIN --force --fsbl images/linux/zynq_fsbl.elf --fpga images/linux/design_1_wrapper.bit --uboot
```

# Petalinux Application
The previous chapters describe the steps to create everything needed to setup the SD card. This SD card then sets up both the OS and the programmable logic (PL) of the board. In this chapter, we describes the control software that is running on the board.

## Features
The Zedboard Software implements both the general protocol implementation and the specific test software to operate our example IoT devices emulated by the STM. The following functionality is given:

* General protocol detection:
    * Basic sending and receiving functions for UART and I2C  
    * Trying I2C protocol with a previous defined address in normal mode
    * Trying UART protocol with 9600 Baud
    * Switching between UART and I2C by Dynamic Reconfiguration until correct protocol is found
* Specific test implementation:
    * Generating LED signals for visualization
    * Reading Button Input for quitting the program
    * Implementing the specific test protocol 
    * Continuously reading a test counter from the IoT device

The general protocol detection routine tries to detect the IoT devices' protocol. After that, the software can operate the IoT device. 

Both the protocol detection and further IoT device usage strongly depends on the IoT device. To test if everything is working, we implemented an example protocol descibed in chapter "Test Implementation".

## XSDK
The software was developed with Xilinx SDK (XSDK). It can be found in the linked folder "srcLinuxApp". 

### First time execution
Note: To establish an ethernet communication, it is recommended to use a network with a DHCP server. Otherwise it is possible to set a static IP in Petalinux.

To start the program on the Zedboard for the first time, a connection to the Zedboard has to be established. For that,  insert the SD card in the Zedboard and turn it on. After that, connect the PC to the Zedboard by the Ethernet socket of the Zedboard. Wait a few minutes so the starting routines of the Zedboard are finished and connect via SSH inside a terminal:

```
ssh root@<IP-Address>
```

Both the username and the password of the Zedboard is "root". After logging in, run XSDK. Create the Zynq device under the "Run Configurations":

Host: <IP-Address>
Port: 1534

After that, create a new targed with this board, the I2C_test3 project and with debug type "Linux Application Debug", and choose "/home/root/iottester" as the remote file path.

Pressing play will now start the software, it can also be debugged now.

### Normal Execution
After the first execution is done, the program can either be started again by pressing play again in XSDK or in the ethernet SSH connection by running the executable in the root-directory:

```
./iottester
```

# IoT Device application
We now have discussed every step to configure and run the Zedboard. In this chapter, we discuss the example IoT devices that we emulate on an STM32 nucleo development board. The software was developed with Keil uVision 5.

## Features
The program implements two example IoT devices that are identical besides their protocol. This protocol can be switched with a button. Two test protocols, I2C (normal mode, address 0x39) and UART (9600 Baud) are implemented.

Their would be many ways to design the communication protocol between Zedboard and STM. The IoT devices orient on the specific protocol implementation described in "Test Implementation". 

The following functionality is given:
* Basic functionality:
    * Timer to generate a secondly counter saved in the IoT device memory
    * I2C and UART functionality
    * Debug-UART via PC
    * LED for protocol visualization
    * Button for protocol switching
* Specific protocol functionality: 
    * IoT device memory that can be accessed by the Zedboard via the protocols
    * Receiving control information from Zedboard
    * Transmitting memory data based on the received control information

The basic idea of this software is to switch between the two protocols to test the Zedboard. Its memory includes the described control information "TUW", a counter, and three additional dummy bytes.

## Operation
To run the software, simply install Keil uVision 5, build and run the project on the STM by connecting it with the USB cable. This cable also serves as the COM port for the debug UART.

# Test Implementation
In this chapter we describe the idea behind the test setup and describe how to use the previously described Zedboard setup with the STM-emulated IoT devices. 

## Basic idea
The Zedboard tries to detect the protocol of the connected IoT device. We use a STM32 nucleo development board to emulate two different IoT devices. Both devices contain a memory containing different data that can be readout but not written from the Zedboard. Among other things, the memory contains a value that counts the seconds the STM is activated. With a button on the STM32, the communication protocol on the same two pins can be switched between UART (9600 BAUD) and I2C (normal mode, slave address 0x39).

The Zedboard tries to detect the communication protocol and then reads the second counter of the STM. If the protocol switches, the Zedboard recognizes this and tries to establish communication again.

## Hardware setup
The following figure shows how to connect the boards and which LEDs and Buttons can be used:

![HardwareSetup](HardwareSetup.jpg)

### Connection
The Zedboard pins JD1_P and JD1_N are connected to the Nucleo-Board's pins PB6 and PB7. Depending on the chosen protocol, they fulfill the following protocol pin functionality:

| UART Mode | I2C Mode | Zedboard | Nucleo Board |
|-----------|----------|----------|--------------|
| Tx        | SDA      | JD1_N    | PB7          |
| Rx        | SCL      | JD1_P    | PB6          |

### Zedboard
On the Zedboard, the BNTC button and the 8 LEDs over the switches are used. 

Pressing BNTC stops the C code.

LED 1-4 show the first 4 bits of the read counter value received from the Nucleo board.
LED 7-8 indicate the chosen protocol. LED 7 is active if UART mode is active, LED 8 is active at active I2C mode.

### STM
On the Nucleo Board, the two buttons and the LED is used.

The blue button switches protocol.
The black button resets the board.

The LED indicates wether UART or I2C mode is activated. An on LED indicates I2C mode, an off LED indicates UART mode.

## Memory
The memory of the IoT device is 7 bytes big and can be addressed via the protocol. It can be written and read by the IoT device but only read by the Zedboard. 

## Protocol
With the protocol the Zedboard is able to request parts of the IoT device memory. For that, it always has to send three bytes:

1. Control sequence "0x7E": The IoT device waits on this control sequence before it starts detecting the protocol.
2. Memory address: The second byte is the address of the starting byte of the memory that are requested. (Do not mistake this with the I2C address). The value can range from 0-6 since it is a 7-byte-Memory.
3. Count: The last byte represents the number of memory bytes that are requested. This can at maximum be n=7-address.

## Protocol detection
The Zedboard needs to have a possibility to recognize if it has chosen the same protocol as the Nucleo Board. In our test implementation, we chose to give the Zedboard the control to detect the right protocol by implementing a special request that the IoT device has to answer. If it does not TODO



## Starting the Test

## Test process

## Activation TODO
For UART connection, open a UART terminal program (like PuTTY: https://www.putty.org/) and wait for the login prompt. 

 To communicate with the IoT device, we send it our specific control byte "0x7E", followed by the memory address of the IoT device ()

