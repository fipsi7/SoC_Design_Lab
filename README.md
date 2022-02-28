# SoC_Design_Lab



# Allgmeines:Projektbeschreibung
wos dama, wos duads, wos geht, ...
+ Anhang: Projektordnerstruktur erklären, und wie man Programm/Projekt verwendet

# Vivado: Projkete, Bilder, Beschreibung, IP's (alle von Xilinx Vivado!)

# Petalinux
Petalinux is a tool suit created by Xilinx to allow the quick generation of a linux kernel running on an Xilinx FPGA. We use it to quickly generate the OS for the dual ARM A9 core inside the ZYNQ XC7Z020-CLG484-1 on the Zedboard.  

We decided to use petalinux because a operating system was needed to provide multitasking as well as scheduling of different IO operations. It also comes with a lot of prebuild kernel drivers for low level connectivity we needed. For example the uart and I2C drivers where already interrupt driven and ready to use. This allowed us to quickly develop the software on a higher level.  

We use the following commands to extract the vivado generated handoff file which contains the prgrammable logic (PL) bitstream as well as the ZYNQ processing system (PS) configuration. This is needed to configure the kernel with the correct parameters as well as for the correct device tree generation. After the hdf extraction, some options where enabled in the petalinux configmenu which are checked in, so we don't go into detail.   
The second command actually builds the runnable image. This takes some time, so feel free to take a coffee break. :D  
Finally the packaging will generate a `BOOT.bin` as well as a `image.ub` which then need to be copied to a FAT formatted SD card.
```
petalinux-config --get-hw-description=../<FOLDER_TO_HDF>
petalinux-build
petalinux-package --boot --format BIN --force --fsbl images/linux/zynq_fsbl.elf --fpga images/linux/design_1_wrapper.bit --uboot
```

# Zedboard Software
The previous chapters describe the steps to create everything needed to setup the SD card. This SD card then sets up both the OS and the programmable logic (PL) of the board. In this chapter, we describes the control software that is running on the board.

## Features
The Zedboard Software implements both the general protocol implementation and the specific test software to operate our example IoT devices emulated by the STM. The following functionality is g iven:

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
To start the program on the Zedboard for the first time, a connection to the Zedboard has to be established. For that,  insert the SD card in the Zedboard and turn it on. After that, connect the PC to the Zedboard by the Ethernet socket of the Zedboard. Wait a few minutes so the starting routines of the Zedboard are finished and connect via SSH inside a terminal:

```
ssh root@192.168.137.2
```

Both the username and the password of the Zedboard is "root". After logging in, run XSDK. Create the Zynq device under the "Run Configurations":

Host: 192.168.137.2
Port: 1534

After that, create a new targed with this board, the I2C_test3 project and with debug type "Linux Application Debug", and choose "/home/root/iottester" as the remote file path.

Pressing play will now start the software, it can also be debugged now.

### Normal Execution
After the first execution is done, the program can either be started again by pressing play again in XSDK or in the ethernet SSH connection by running the executable in the root-directory:

```
./iottester
```

# IoT device emulation
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
TODO


