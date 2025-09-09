/**************************************************************************************************
 ** file         : libusb-vbat.c
 ** description  : print current battery voltage of connected mouse
 **
 ** compilation  : gcc libusb-vbat.c -lusb-1.0 -o libusb-vbat
 **
 ** permissions  : create a rules file, e.g., `/etc/udev/rules.d/99-hiiri.rules`
 **                and write: 
 **                SUBSYSTEM=="usb", ATTR{idVendor}=="1915", ATTR{idProduct}=="572b", MODE="0666"
 **
 ** usage        : ./libusb-vbat
 **
 *************************************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

int main(int argc, char **argv) {
    
    libusb_context *ctx = NULL;
    libusb_device_handle *dev_handle = NULL;
    int ret;
    uint8_t vbat_step = 0;
    uint16_t vbat_mv = 0;

    ret = libusb_init_context(&ctx, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Failed to initialize libusb\n");
        return 1;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, 0x1915, 0x572B);
    if (dev_handle == NULL) {
        fprintf(stderr, "Error: cannot open device 0x1915:0x572B\n");
        libusb_exit(ctx);
        return 1;
    }

    ret = libusb_control_transfer(dev_handle, 0b11000000, 0x01, 0, 0, &vbat_step, 1, 100);
    if (ret < 0) {
        fprintf(stderr, "Error: control transfer error: %s\n", libusb_strerror(ret));
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return 1;
    }

    libusb_close(dev_handle);
    libusb_exit(ctx);

    /* mask out button bits [1:0] */
    vbat_step = (vbat_step & 0b11111100) >> 2;
    printf("vbat_step: %d\n", vbat_step);

    /* convert ref ladder step to mV */
    vbat_mv = ((vbat_step + 1) * 1200U * 5U) / 64U;
    printf("vbat_mv: %dmV\n", vbat_mv);

    return 0;
}
