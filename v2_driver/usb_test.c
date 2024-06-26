#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "header.h"

GtkWidget *entry_input_filename;
GtkWidget *entry_output_filename;
GtkWidget *entry_custom_string;
void process_data(const char *input_filename, const char *output_filename, const char *custom_string, int mode)
{
    int fd;
    struct ioctl_data data;
    // Mở thiết bị
    fd = open("/dev/usb_crypto", O_RDWR);
    if (fd == -1)
    {
        perror("Failed to open the device");
        printf("Hay cam usb de co the tiep tuc\n");
        exit(1);
    }
    strncpy(data.input_filename, input_filename, MAX_FILENAME_LEN);
    strncpy(data.output_filename, output_filename, MAX_FILENAME_LEN);
    strncpy(data.custom_string, custom_string, MAX_STRING_LEN);
    data.custom_string[MAX_STRING_LEN] = '\0';

    // Gọi ioctl để thiết lập chế độ
    if (mode == 1)
    {
        printf("Ma hoa du lieu\n");
        ioctl(fd, IOCTL_ENCRYPT, &data);
    }
    else if (mode == 2)
    {
        printf("Giai ma du lieu\n");
        ioctl(fd, IOCTL_DECRYPT, &data);
    }
    printf("Ket thuc cong viec xu li du lieu\n");

    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input file> <output file> <mode: 1-encrypt, 2-decrypt> <filename>\n", argv[0]);
        return 1;
    }
    int mode = atoi(argv[1]);
    char *filename = argv[2];
    printf("gia tri mode: %d\n", mode);

    process_data(filename, mode);
    return 0;
}
