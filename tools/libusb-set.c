/********************************************************************
 ** file         : libusb-set.c
 ** description  : set dpi and polling rate
 **
 ** compilation  : gcc libusb-set.c -lusb-1.0 -o libusb-set
 **
 ** permissions  : create a rules file, e.g., `/etc/udev/rules.d/99-hiiri.rules`
 **                and write: 
 **                SUBSYSTEM=="usb", ATTR{idVendor}=="1915", ATTR{idProduct}=="572b", MODE="0666"
 **
 ** usage        : ./libusb-set <dpi> <binterval>
 **
 *******************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

int main(int argc, char **argv) {
    
    libusb_context *ctx = NULL;
    libusb_device_handle *dev_handle = NULL;
    int ret;
    uint16_t dpi;
    uint16_t binterval;

    if (argc != 3) {
        fprintf(stderr, "Usage: ./libusb-set <dpi> <binterval> \n");
        return 1;
    }
    else {
        dpi       = atoi(argv[1]);
        binterval = atoi(argv[2]);
    }

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

    ret = libusb_control_transfer(dev_handle, 0b01000000, 0x01, dpi, binterval, NULL, 0, 100);
    if (ret < 0) {
        fprintf(stderr, "Error: control transfer error: %s\n", libusb_strerror(ret));
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return 1;
    }

    printf("Successfully sent `set_mousesettings` request: dpi = %d, binterval = %d\n", dpi, binterval);
    libusb_close(dev_handle);
    libusb_exit(ctx);

    return 0;
}
