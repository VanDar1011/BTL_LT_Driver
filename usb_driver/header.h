#ifndef USB_CRYPTO_IOCTL_H
#define USB_CRYPTO_IOCTL_H

#include <linux/ioctl.h>

// Kích thước bộ đệm (tùy chỉnh theo nhu cầu của bạn)
#define BUFFER_SIZE 256

// Định nghĩa mã lệnh IOCTL
#define IOCTL_ENCRYPT _IO('k', 1)
#define IOCTL_DECRYPT _IO('k', 2)

#endif // USB_CRYPTO_IOCTL_H

