#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "header.h"

void process_data(int mode)
{
    int fd;
    // Mở thiết bị
    fd = open("/dev/usb_crypto", O_RDWR);
    if (fd == -1)
    {
        perror("Failed to open the device");
	printf("Hay cam usb de co the tiep tuc\n");
        exit(1);
    }

    // Gọi ioctl để thiết lập chế độ
    if (mode == 1)
    {
        printf("Ma hoa du lieu\n");
        ioctl(fd, IOCTL_ENCRYPT);
    }
    else if (mode == 2)
    {
        printf("Giai ma du lieu\n");
        ioctl(fd, IOCTL_DECRYPT);
    }
    printf("Ket thuc cong viec xu li du lieu\n");

    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input file> <output file> <mode: 1-encrypt, 2-decrypt>\n", argv[0]);
        return 1;
    }
    int mode = atoi(argv[1]);
    printf("gia tri mode: %d\n",mode);

    process_data(mode);
    return 0;
}
