#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <dirent.h> // Thư viện để đọc thư mục
#include "header.h"

GtkWidget *entry_input_filename;
GtkWidget *entry_output_filename;
GtkWidget *entry_custom_string;

// Hàm xử lý dữ liệu
void process_data(const char *input_filename, const char *output_filename, const char *custom_string, int mode)
{
    int fd;
    struct ioctl_data data;

    // Mở thiết bị
    fd = open("/dev/usb_crypto", O_RDWR);
    if (fd == -1)
    {
        perror("Failed to open the device");
        printf("Please insert the USB device to continue\n");
        exit(1);
    }

    // Gọi ioctl để thiết lập chế độ và truyền tên file
    strncpy(data.input_filename, input_filename, BUFFER_SIZE);
    strncpy(data.output_filename, output_filename, BUFFER_SIZE);
    strncpy(data.custom_string, custom_string, 16);
    data.custom_string[16] = '\0';
    if (mode == 1)
    {
        printf("Encrypting fileInput: %s\n", data.input_filename);
        printf("Encrypting fileOutput: %s\n", data.output_filename);
        printf("Encrypting data: %s\n", data.custom_string);
        ioctl(fd, IOCTL_ENCRYPT, &data);
    }
    else if (mode == 2)
    {
        printf("Encrypting fileInput: %s\n", data.input_filename);
        printf("Encrypting fileOutput: %s\n", data.output_filename);
        printf("Encrypting data: %s\n", data.custom_string);
        ioctl(fd, IOCTL_DECRYPT, &data);
    }
    printf("Data processing finished\n");

    close(fd);
}

// Callback khi nhấn nút chọn file
static void on_select_input_file_button_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Select Input File",
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
        gtk_entry_set_text(GTK_ENTRY(entry_input_filename), filename);
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
    const gchar *input_filename = gtk_entry_get_text(GTK_ENTRY(entry_input_filename));
    const gchar *output_filename = gtk_entry_get_text(GTK_ENTRY(entry_output_filename));
    const gchar *custom_string = gtk_entry_get_text(GTK_ENTRY(entry_custom_string));

    if (strlen(input_filename) == 0 || strlen(output_filename) == 0 || strlen(custom_string) == 0)
    {
        g_print("Please select input and output files, and enter custom string.\n");
        return;
    }

    process_data(input_filename, output_filename, custom_string, 1);
}

// Callback cho nút giải mã
static void on_decrypt_button_clicked(GtkWidget *widget, gpointer data)
{
    const gchar *input_filename = gtk_entry_get_text(GTK_ENTRY(entry_input_filename));
    const gchar *output_filename = gtk_entry_get_text(GTK_ENTRY(entry_output_filename));
    const gchar *custom_string = gtk_entry_get_text(GTK_ENTRY(entry_custom_string));

    if (strlen(input_filename) == 0 || strlen(output_filename) == 0 || strlen(custom_string) == 0)
    {
        g_print("Please select input and output files, and enter custom string.\n");
        return;
    }

    process_data(input_filename, output_filename, custom_string, 2);
}

// Chương trình chính
int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *encrypt_button;
    GtkWidget *decrypt_button;
    GtkWidget *select_input_file_button;
    GtkWidget *select_output_file_button;
    GtkWidget *label_custom_string;

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

    // Nhãn cho nhập tên file input
    GtkWidget *label_input_filename = gtk_label_new("Input File:");
    gtk_grid_attach(GTK_GRID(grid), label_input_filename, 0, 0, 1, 1);

    // Ô nhập tên file input
    entry_input_filename = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_input_filename, 1, 0, 1, 1);

    // Tạo nút chọn file input
    select_input_file_button = gtk_button_new_with_label("Select Input File");
    gtk_grid_attach(GTK_GRID(grid), select_input_file_button, 2, 0, 1, 1);
    g_signal_connect(select_input_file_button, "clicked", G_CALLBACK(on_select_input_file_button_clicked), window);

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

    // Nhãn cho nhập chuỗi tùy chỉnh
    label_custom_string = gtk_label_new("Custom String (1-16 chars):");
    gtk_grid_attach(GTK_GRID(grid), label_custom_string, 0, 2, 1, 1);

    // Ô nhập chuỗi tùy chỉnh
    entry_custom_string = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_custom_string, 1, 2, 1, 1);

    // Tạo nút mã hóa
    encrypt_button = gtk_button_new_with_label("Encrypt");
    g_signal_connect(encrypt_button, "clicked", G_CALLBACK(on_encrypt_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), encrypt_button, 0, 3, 1, 1);

    // Tạo nút giải mã
    decrypt_button = gtk_button_new_with_label("Decrypt");
    g_signal_connect(decrypt_button, "clicked", G_CALLBACK(on_decrypt_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), decrypt_button, 1, 3, 1, 1);

    // Hiển thị tất cả các widget
    gtk_widget_show_all(window);

    // Kết nối sự kiện đóng cửa sổ với gtk_main_quit để thoát ứng dụng
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Vòng lặp chính của GTK
    gtk_main();

    return 0;
}