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

# SW: STM, XSDK (also auf Petalinux), dyn. Rekonfigurieren!


