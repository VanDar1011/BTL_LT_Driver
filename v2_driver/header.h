#ifndef USB_CRYPTO_IOCTL_H
#define USB_CRYPTO_IOCTL_H

#include <linux/ioctl.h>

// Kích thước bộ đệm (tùy chỉnh theo nhu cầu của bạn)
#define BUFFER_SIZE 256
struct ioctl_data
{
    char input_filename[BUFFER_SIZE];
    char output_filename[BUFFER_SIZE];
    char custom_string[17]; // +1 để chứa ký tự kết thúc chuỗi '\0'
};
// Định nghĩa mã lệnh IOCTL
#define IOCTL_ENCRYPT _IOW('k', 1, struct ioctl_data *)
#define IOCTL_DECRYPT _IOW('k', 2, struct ioctl_data *)

#endif // USB_CRYPTO_IOCTL_H
