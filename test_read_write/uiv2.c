#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <dirent.h> // Thư viện để đọc thư mục
#include "header.h"

GtkWidget *entry_filename;
GtkWidget *entry_output_filename;

// Hàm xử lý dữ liệu với file output
void process_data(const char *filename, int mode)
{
    int fd;
    char file_name[MAX_FILENAME_LEN];

    // Mở thiết bị
    fd = open("/dev/usb_crypto", O_RDWR);
    if (fd == -1)
    {
        perror("Failed to open the device");
        printf("Please insert the USB device to continue\n");
        exit(1);
    }

    // Gọi ioctl để thiết lập chế độ và truyền tên file
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

// Callback khi nhấn nút chọn file
static void on_select_file_button_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Select File",
                                         GTK_WINDOW(data),
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
        gtk_entry_set_text(GTK_ENTRY(entry_filename), filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Callback khi nhấn nút chọn file output
static void on_select_output_file_button_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Select Output File",
                                         GTK_WINDOW(data),
                                         action,
                                         ("_Cancel"),
                                         GTK_RESPONSE_CANCEL,
                                         ("_Save"),
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        gtk_entry_set_text(GTK_ENTRY(entry_output_filename), filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Callback cho nút mã hóa
static void on_encrypt_button_clicked(GtkWidget *widget, gpointer data)
{
    const gchar *filename = gtk_entry_get_text(GTK_ENTRY(entry_filename));
    const gchar *output_filename = gtk_entry_get_text(GTK_ENTRY(entry_output_filename));

    if (strlen(filename) == 0 || strlen(output_filename) == 0)
    {
        g_print("Please select both input and output files.\n");
        return;
    }

    char file_name[MAX_FILENAME_LEN];
    snprintf(file_name, MAX_FILENAME_LEN, "%s;%s", filename, output_filename);
    process_data(file_name, 1);
}

// Callback cho nút giải mã
static void on_decrypt_button_clicked(GtkWidget *widget, gpointer data)
{
    const gchar *filename = gtk_entry_get_text(GTK_ENTRY(entry_filename));
    const gchar *output_filename = gtk_entry_get_text(GTK_ENTRY(entry_output_filename));

    if (strlen(filename) == 0 || strlen(output_filename) == 0)
    {
        g_print("Please select both input and output files.\n");
        return;
    }

    char file_name[MAX_FILENAME_LEN];
    snprintf(file_name, MAX_FILENAME_LEN, "%s;%s", filename, output_filename);
    process_data(file_name, 2);
}

// Chương trình chính
int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *encrypt_button;
    GtkWidget *decrypt_button;
    GtkWidget *select_file_button;
    GtkWidget *select_output_file_button;

    gtk_init(&argc, &argv);

    // Tạo cửa sổ chính
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "USB Crypto");
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    // Đặt cửa sổ vào giữa màn hình
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    // Tạo lưới để chứa các widget
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Nhãn cho nhập tên file
    GtkWidget *label_filename = gtk_label_new("Selected File:");
    gtk_grid_attach(GTK_GRID(grid), label_filename, 0, 0, 1, 1);

    // Ô nhập tên file
    entry_filename = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_filename, 1, 0, 1, 1);

    // Tạo nút chọn file
    select_file_button = gtk_button_new_with_label("Select File");
    gtk_grid_attach(GTK_GRID(grid), select_file_button, 2, 0, 1, 1);
    g_signal_connect(select_file_button, "clicked", G_CALLBACK(on_select_file_button_clicked), window);

    // Nhãn cho nhập tên file output
    GtkWidget *label_output_filename = gtk_label_new("Output File:");
    gtk_grid_attach(GTK_GRID(grid), label_output_filename, 0, 1, 1, 1);

    // Ô nhập tên file output
    entry_output_filename = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_output_filename, 1, 1, 1, 1);

    // Tạo nút chọn file output
    select_output_file_button = gtk_button_new_with_label("Select Output File");
    gtk_grid_attach(GTK_GRID(grid), select_output_file_button, 2, 1, 1, 1);
    g_signal_connect(select_output_file_button, "clicked", G_CALLBACK(on_select_output_file_button_clicked), window);

    // Tạo nút mã hóa
    encrypt_button = gtk_button_new_with_label("Encrypt");
    g_signal_connect(encrypt_button, "clicked", G_CALLBACK(on_encrypt_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), encrypt_button, 0, 2, 1, 1);

    // Tạo nút giải mã
    decrypt_button = gtk_button_new_with_label("Decrypt");
    g_signal_connect(decrypt_button, "clicked", G_CALLBACK(on_decrypt_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), decrypt_button, 1, 2, 1, 1);

    // Hiển thị tất cả các widget
    gtk_widget_show_all(window);

    // Kết nối sự kiện đóng cửa sổ với gtk_main_quit để thoát ứng dụng
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Vòng lặp chính của GTK
    gtk_main();

    return 0;
}
