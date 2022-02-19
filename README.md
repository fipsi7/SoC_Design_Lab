# SoC_Design_Lab



# Allgmeines:Projektbeschreibung
wos dama, wos duads, wos geht, ...
+ Anhang: Projektordnerstruktur erklären, und wie man Programm/Projekt verwendet

# Vivado: Projkete, Bilder, Beschreibung, IP's (alle von Xilinx Vivado!)

# Petalinux: warum, wie, was macht das?
a kurz beschreiben, welche Befehle
```
petalinux-config --get-hw-description=../<FOLDER_TO_HDF>
petalinux-build
petalinux-package --boot --format BIN --force --fsbl images/linux/zynq_fsbl.elf --fpga images/linux/design_1_wrapper.bit --uboot
```

# SW: STM, XSDK (also auf Petalinux), dyn. Rekonfigurieren!


