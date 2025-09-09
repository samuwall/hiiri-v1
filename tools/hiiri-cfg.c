/********************************************************************
 ** file         : hiiri-cfg.c
 ** description  : simple gtk4 GUI for setting DPI / poll mode
 **
 ** compilation  : gcc -o hiiri-cfg hiiri-cfg.c `pkg-config --cflags --libs gtk4 libusb-1.0`
 **
 ** permissions  : create a rules file, e.g., `/etc/udev/rules.d/99-hiiri.rules`
 **                and write: 
 **                SUBSYSTEM=="usb", ATTR{idVendor}=="1915", ATTR{idProduct}=="572b", MODE="0666"
 **
 ** usage        : ./hiiri-cfg
 **
 *******************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <libusb-1.0/libusb.h>

#define USB_VID 0x1915
#define USB_PID 0x572B

static GtkWidget *dpi_scale;
static GtkWidget *bat_label;
static GtkWidget *dpi_label;
static GtkWidget *cur_dpi_label;
static uint8_t poll_mode = 0;

static uint16_t my_pow(uint8_t base, uint8_t exponent) {
    uint16_t result = 1;
    for (uint8_t i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

static uint8_t pollmode_to_binterval(uint8_t mode) {
    static const uint8_t map[] = {0, 8, 4, 2, 1};
    return (mode < (sizeof(map) / sizeof(map[0]))) ? map[mode] : 0;
}

static void update_dpi_label(GtkRange *range) {

    uint16_t dpi = (400 * my_pow(2, (uint8_t)gtk_range_get_value(range)));
    char label_text[50];
    snprintf(label_text, sizeof(label_text), "Selected DPI: %d", dpi);
    gtk_label_set_text(GTK_LABEL(dpi_label), label_text);

}

static void poll_button_toggled(GtkWidget *widget, gpointer data) {

    (void)data;
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(widget))) {
        poll_mode = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "index"));
    }

}

static void show_error_dialog(GtkWindow *parent, const char *message) {

    const char *btn_labels[] = {"OK", NULL};
    GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", message);
    gtk_alert_dialog_set_buttons(dialog, btn_labels);
    gtk_alert_dialog_show(dialog, parent);
    g_object_unref(dialog);

}

static void show_success_dialog(GtkWindow *parent, uint16_t dpi, uint16_t poll_mode) {

    const char *btn_labels[] = {"OK", NULL};
    char msg[100];
    snprintf(msg, sizeof(msg), "Successfully sent `set_mousesettings()` request.\n"
                                "dpi: %d\nbInterval: %d", dpi, poll_mode);
    GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", msg);
    gtk_alert_dialog_set_buttons(dialog, btn_labels);
    gtk_alert_dialog_show(dialog, parent);
    g_object_unref(dialog);

}

static gboolean update_vbat_label(gpointer data) {

    (void)data;
    libusb_context *ctx = NULL;
    libusb_device_handle *dev_handle = NULL;
    uint8_t vbat_step = 0;
    float vbat = 0;
    int ret;
    
    ret = libusb_init_context(&ctx, NULL, 0);
    if (ret < 0) {
        gtk_label_set_text(GTK_LABEL(bat_label), "Battery: N/A");
        return G_SOURCE_CONTINUE;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, USB_VID, USB_PID);
    if (!dev_handle) {
        gtk_label_set_text(GTK_LABEL(bat_label), "Battery: N/A");
        libusb_exit(ctx);
        return G_SOURCE_CONTINUE;
    }

    ret = libusb_control_transfer(dev_handle, 0xC0, 0x01, 0, 0, &vbat_step, 1, 100);
    if (ret < 0) {
        printf("%s\n", libusb_strerror(ret));
        gtk_label_set_text(GTK_LABEL(bat_label), "Battery: N/A");
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return G_SOURCE_CONTINUE;
    }

    libusb_close(dev_handle);
    libusb_exit(ctx);

    /* mask out button bits [1:0], convert to V */
    vbat_step = (vbat_step & 0b11111100) >> 2;
    vbat = ((vbat_step + 1) * 1.2 * 5) / 64;

    char label_text[64];
    snprintf(label_text, sizeof(label_text), "Battery: %1.4fV", vbat);
    gtk_label_set_text(GTK_LABEL(bat_label), label_text);

    /* keep ticking */
    return G_SOURCE_CONTINUE;

}

static void submit_clicked(GtkWidget *widget, gpointer data) {

    (void)data;
    uint16_t dpi = (400 * my_pow(2, (uint8_t)gtk_range_get_value(GTK_RANGE(dpi_scale))));
    uint8_t bInterval = pollmode_to_binterval(poll_mode);
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_native(widget));
    libusb_context *ctx = NULL;
    libusb_device_handle *dev_handle = NULL;
    int ret;

    printf("dpi: %d\nbInterval: %d\n", dpi, bInterval);

    ret = libusb_init_context(&ctx, NULL, 0);
    if (ret < 0) {
        show_error_dialog(parent, "Failed to initialize libusb");
        return;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, USB_VID, USB_PID);
    if (!dev_handle) {
        show_error_dialog(parent, "Could not open device\nCheck if mouse is connected");
        libusb_exit(ctx);
        return;
    }

    ret = libusb_control_transfer(dev_handle, 0x40, 0x01, dpi, bInterval, NULL, 0, 100);
    if (ret < 0) {
        show_error_dialog(parent, libusb_strerror(ret));
    } else {
        show_success_dialog(parent, dpi, bInterval);
    }

    libusb_close(dev_handle);
    libusb_exit(ctx);
}

static void activate(GtkApplication *app, gpointer data) {

    (void)data;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "hiiri-cfg");
    gtk_window_set_default_size(GTK_WINDOW(window), 320, 180);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    /* mouse status */
    GtkWidget *status_row = gtk_center_box_new();
    gtk_box_append(GTK_BOX(vbox), status_row);

    bat_label = gtk_label_new("Battery: —");
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(status_row), bat_label);

    cur_dpi_label = gtk_label_new("Current DPI: 800");
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(status_row), cur_dpi_label);

    /* dpi frame */
    GtkWidget *dpi_frame = gtk_frame_new("DPI");
    gtk_box_append(GTK_BOX(vbox), dpi_frame);
    gtk_widget_set_vexpand(dpi_frame, TRUE);
    
    GtkWidget *dpi_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_vexpand(dpi_content, TRUE);
    gtk_frame_set_child(GTK_FRAME(dpi_frame), dpi_content);
    
    dpi_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 6, 1);
    gtk_range_set_value(GTK_RANGE(dpi_scale), 1);
    gtk_widget_set_hexpand(dpi_scale, TRUE);
    g_signal_connect(dpi_scale, "value-changed", G_CALLBACK(update_dpi_label), NULL);
    gtk_box_append(GTK_BOX(dpi_content), dpi_scale);
    
    dpi_label = gtk_label_new("Selected DPI: 800");
    gtk_box_append(GTK_BOX(dpi_content), dpi_label);

    /* pollrate frame */
    GtkWidget *poll_frame = gtk_frame_new("Polling Rate");
    gtk_box_append(GTK_BOX(vbox), poll_frame);
    gtk_widget_set_vexpand(poll_frame, TRUE);
    
    GtkWidget *poll_grid = gtk_grid_new();
    gtk_widget_set_vexpand(poll_grid, TRUE);
    gtk_widget_set_hexpand(poll_grid, TRUE);
    gtk_frame_set_child(GTK_FRAME(poll_frame), poll_grid);
    
    const char *poll_labels[] = {"N/A", "125Hz", "250Hz", "500Hz", "1000Hz"};
    GtkCheckButton *group_leader = NULL;
    
    for (int i = 0; i < 5; i++) {
        GtkWidget *check = gtk_check_button_new_with_label(poll_labels[i]);
        g_object_set_data(G_OBJECT(check), "index", GINT_TO_POINTER(i));
        g_signal_connect(check, "toggled", G_CALLBACK(poll_button_toggled), NULL);
        gtk_grid_attach(GTK_GRID(poll_grid), check, i, 0, 1, 1);
        
        if (i == 0) {
            group_leader = GTK_CHECK_BUTTON(check);
            gtk_check_button_set_active(GTK_CHECK_BUTTON(check), TRUE);
        } else {
            gtk_check_button_set_group(GTK_CHECK_BUTTON(check), group_leader);
        }
    }

    /* submit */
    GtkWidget *submit_btn = gtk_button_new_with_label("Apply Settings");
    g_signal_connect(submit_btn, "clicked", G_CALLBACK(submit_clicked), NULL);
    gtk_box_append(GTK_BOX(vbox), submit_btn);

    /* make visible */
    gtk_widget_set_visible(window, TRUE);

    /* start updating vbat */
    update_vbat_label(NULL);
    g_timeout_add_seconds(15, update_vbat_label, NULL);

}

int main(int argc, char **argv) {

    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.mouseconfig", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
