#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "usb_crypto_ioctl.h"

// Hàm xử lý dữ liệu
void process_data(const char *filename, int mode)
{
    int fd;
    // Mở thiết bị
    fd = open("/dev/usb_crypto", O_RDWR);
    if (fd == -1)
    {
        perror("Failed to open the device");
        printf("Please insert the USB device to continue\n");
        exit(1);
    }

    // Gọi ioctl để thiết lập chế độ
    if (mode == 1)
    {
        printf("Encrypting data: %s\n", filename);
        ioctl(fd, IOCTL_ENCRYPT, filename);
    }
    else if (mode == 2)
    {
        printf("Decrypting data: %s\n", filename);
        ioctl(fd, IOCTL_DECRYPT, filename);
    }
    printf("Data processing finished\n");

    close(fd);
}

// Callback cho nút mã hóa
static void on_encrypt_button_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Choose File to Encrypt",
                                         NULL,
                                         action,
                                         ("_Cancel"),
                                         GTK_RESPONSE_CANCEL,
                                         ("_Open"),
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        process_data(filename, 1);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Callback cho nút giải mã
static void on_decrypt_button_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Choose File to Decrypt",
                                         NULL,
                                         action,
                                         ("_Cancel"),
                                         GTK_RESPONSE_CANCEL,
                                         ("_Open"),
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        process_data(filename, 2);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Chương trình chính
int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *encrypt_button;
    GtkWidget *decrypt_button;

    gtk_init(&argc, &argv);

    // Tạo cửa sổ chính
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "USB Crypto");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);

    // Tạo lưới để chứa các widget
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Tạo nút mã hóa
    encrypt_button = gtk_button_new_with_label("Encrypt");
    g_signal_connect(encrypt_button, "clicked", G_CALLBACK(on_encrypt_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), encrypt_button, 0, 0, 1, 1);

    // Tạo nút giải mã
    decrypt_button = gtk_button_new_with_label("Decrypt");
    g_signal_connect(decrypt_button, "clicked", G_CALLBACK(on_decrypt_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), decrypt_button, 1, 0, 1, 1);

    // Hiển thị tất cả các widget
    gtk_widget_show_all(window);

    // Kết nối sự kiện đóng cửa sổ với gtk_main_quit để thoát ứng dụng
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Vòng lặp chính của GTK
    gtk_main();

    return 0;
}
