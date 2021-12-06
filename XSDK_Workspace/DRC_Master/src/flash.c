/*
 * flash.c
 *
 *  Created on: 06.12.2021
 *      Author: andi
 */


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define PATH "/home/root/%s"
//#define OLD "/home/root/%s"

int flash_bitfile(char *name){
    char file_name[256];
    int fd;
    int ret;
    int size = 5000000;
    char buffer[5000000] = "";

    if (name != '\0') {
        // compose file name
        sprintf(file_name, PATH, name);

        // open partial bitfile
        fd = open(file_name, O_RDONLY);
        if (fd < 0) {
            printf("failed to open partial bitfile %s\n", file_name);
            fflush(stdout);
            return -1;
        }

        // read partial bitfile into buffer
        ret = read(fd, buffer, size);
        if (ret < 0) {
            printf("failed to read partial bitfile %s\n", file_name);
            fflush(stdout);
            close(fd);
            return -1;
        }

        // close file handle
        close(fd);

  // Write partial bitfile to xdevcfg device
  fd = open("/dev/xdevcfg", O_RDWR);
  if (fd < 0) {
   printf("failed to open xdevcfg device\n");
   fflush(stdout);
   return -1;
  }
  write(fd, buffer, size);

  //if(rename_en == 0){
  // char temp[256];
  // sprintf(temp, OLD, name);
  // rename(file_name, temp);
  //}

  close(fd);

  int done = open("/sys/devices/soc0/amba/f8007000.devcfg/prog_done", O_RDWR);
  if (done < 0) {
   printf("failed to get xdevcfg attribute 'prog_done'\n");
   fflush(stdout);
   return -1;
  }
  char is_set[2];
  read(done, is_set, 2);
  printf("Prog_Done: %s", is_set);

  close(done);
    }

    return 0;
}
