#ifndef USB_CRYPTO_IOCTL_H
#define USB_CRYPTO_IOCTL_H

#include <linux/ioctl.h>

// Kích thước bộ đệm (tùy chỉnh theo nhu cầu của bạn)
#define BUFFER_SIZE 256
#define MAX_FILENAME_LEN 256
// Định nghĩa mã lệnh IOCTL
#define IOCTL_ENCRYPT _IOW('k', 1, char[MAX_FILENAME_LEN])
#define IOCTL_DECRYPT _IOW('k', 2, char[MAX_FILENAME_LEN])

#endif // USB_CRYPTO_IOCTL_H

