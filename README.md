# SoC_Design_Lab

```
petalinux-config --get-hw-description=../<FOLDER_TO_HDF>
petalinux-build
petalinux-package --boot --fsbl image/linux/zynq.elf --fpga image/linux/top.bit --u-boot
```
