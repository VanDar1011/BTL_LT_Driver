#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h> // struct usb_device
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/device.h> // struct device
#include "header.h"
// Thông tin về device USB mà driver này hỗ trợ
#define VENDOR_ID 0x21c4
#define PRODUCT_ID 0x0cd1
#define DEVICE_NAME "usb_crypto"
#define CLASS_NAME "usb_class"
//
#define USB_DEVICE_ID "4-1:1.0"
//
#define MY_MAJOR 42
#define MY_MINOR 0
#define NUM_MINORS 1
#define BUF_SIZE_USB 4096
//
#define BUF_SIZE 16
unsigned char key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
unsigned char expandedKey[1000];
static char *file_a = "plaintext_input.txt"; // Path to input file
static char *file_b = "plaintext_output.txt";
static char *file_c = "ciphertext_output.txt";
static char file_name[MAX_FILENAME_LEN] = {0};
//
static struct usb_device *crypto_usb_device;
static struct device *crypto_device;
static struct class *class;
static struct cdev cdev;            // manager character device
static dev_t *crypto_device_number; // device major, minor

static char device_buffer[BUF_SIZE_USB] = {0};
static int device_buffer_len = 0;
static int device_read_complete = 0;
static struct usb_driver simple_usb_driver;

static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_write(struct file *file, const char __user *buf, size_t count, loff_t *pos);
static ssize_t device_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
// ubind default

unsigned char sBox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};
unsigned char invSBox[256] = {
    0x52,
    0x09,
    0x6a,
    0xd5,
    0x30,
    0x36,
    0xa5,
    0x38,
    0xbf,
    0x40,
    0xa3,
    0x9e,
    0x81,
    0xf3,
    0xd7,
    0xfb,
    0x7c,
    0xe3,
    0x39,
    0x82,
    0x9b,
    0x2f,
    0xff,
    0x87,
    0x34,
    0x8e,
    0x43,
    0x44,
    0xc4,
    0xde,
    0xe9,
    0xcb,
    0x54,
    0x7b,
    0x94,
    0x32,
    0xa6,
    0xc2,
    0x23,
    0x3d,
    0xee,
    0x4c,
    0x95,
    0x0b,
    0x42,
    0xfa,
    0xc3,
    0x4e,
    0x08,
    0x2e,
    0xa1,
    0x66,
    0x28,
    0xd9,
    0x24,
    0xb2,
    0x76,
    0x5b,
    0xa2,
    0x49,
    0x6d,
    0x8b,
    0xd1,
    0x25,
    0x72,
    0xf8,
    0xf6,
    0x64,
    0x86,
    0x68,
    0x98,
    0x16,
    0xd4,
    0xa4,
    0x5c,
    0xcc,
    0x5d,
    0x65,
    0xb6,
    0x92,
    0x6c,
    0x70,
    0x48,
    0x50,
    0xfd,
    0xed,
    0xb9,
    0xda,
    0x5e,
    0x15,
    0x46,
    0x57,
    0xa7,
    0x8d,
    0x9d,
    0x84,
    0x90,
    0xd8,
    0xab,
    0x00,
    0x8c,
    0xbc,
    0xd3,
    0x0a,
    0xf7,
    0xe4,
    0x58,
    0x05,
    0xb8,
    0xb3,
    0x45,
    0x06,
    0xd0,
    0x2c,
    0x1e,
    0x8f,
    0xca,
    0x3f,
    0x0f,
    0x02,
    0xc1,
    0xaf,
    0xbd,
    0x03,
    0x01,
    0x13,
    0x8a,
    0x6b,
    0x3a,
    0x91,
    0x11,
    0x41,
    0x4f,
    0x67,
    0xdc,
    0xea,
    0x97,
    0xf2,
    0xcf,
    0xce,
    0xf0,
    0xb4,
    0xe6,
    0x73,
    0x96,
    0xac,
    0x74,
    0x22,
    0xe7,
    0xad,
    0x35,
    0x85,
    0xe2,
    0xf9,
    0x37,
    0xe8,
    0x1c,
    0x75,
    0xdf,
    0x6e,
    0x47,
    0xf1,
    0x1a,
    0x71,
    0x1d,
    0x29,
    0xc5,
    0x89,
    0x6f,
    0xb7,
    0x62,
    0x0e,
    0xaa,
    0x18,
    0xbe,
    0x1b,
    0xfc,
    0x56,
    0x3e,
    0x4b,
    0xc6,
    0xd2,
    0x79,
    0x20,
    0x9a,
    0xdb,
    0xc0,
    0xfe,
    0x78,
    0xcd,
    0x5a,
    0xf4,
    0x1f,
    0xdd,
    0xa8,
    0x33,
    0x88,
    0x07,
    0xc7,
    0x31,
    0xb1,
    0x12,
    0x10,
    0x59,
    0x27,
    0x80,
    0xec,
    0x5f,
    0x60,
    0x51,
    0x7f,
    0xa9,
    0x19,
    0xb5,
    0x4a,
    0x0d,
    0x2d,
    0xe5,
    0x7a,
    0x9f,
    0x93,
    0xc9,
    0x9c,
    0xef,
    0xa0,
    0xe0,
    0x3b,
    0x4d,
    0xae,
    0x2a,
    0xf5,
    0xb0,
    0xc8,
    0xeb,
    0xbb,
    0x3c,
    0x83,
    0x53,
    0x99,
    0x61,
    0x17,
    0x2b,
    0x04,
    0x7e,
    0xba,
    0x77,
    0xd6,
    0x26,
    0xe1,
    0x69,
    0x14,
    0x63,
    0x55,
    0x21,
    0x0c,
    0x7d,

};
// Define subByte,invSubBytes
void subBytes(unsigned char *state)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        // Lấy giá trị hiện tại của state và sử dụng nó làm chỉ số trong S-box
        state[i] = sBox[state[i]];
    }
}
void invSubBytes(unsigned char *state)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        // Lấy giá trị hiện tại của state và sử dụng nó làm chỉ số trong S-box ngược lại
        state[i] = invSBox[state[i]];
    }
}

// Function to perform AddRoundKey operation

unsigned char Rcon[40] = {
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x36};

void SubWord(unsigned char *word)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        word[i] = sBox[word[i]];
    }
}

// Hàm xoay word sang trái một byte
void RotWord(unsigned char *word)
{
    unsigned char temp = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = temp;
}

void ExpandKey(unsigned char *key, unsigned char *expandedKey)
{
    int bytesGenerated = 0;
    int indexRcon = 0;
    // Copy khóa ban đầu vào expanded key
    int k;
    for (k = 0; k < 16; k++)
    {
        expandedKey[k] = key[k];
    }
    bytesGenerated += 16;

    while (bytesGenerated <= 176)
    {
        unsigned char temp[4];
        unsigned char temp1[4];
        //  if(bytesGenerated % 16 == 0)
        int i, j;
        for (i = 0; i < 4; i++)
        {
            temp[i] = temp1[i] = expandedKey[i + bytesGenerated - 4];
        }

        RotWord(temp);
        SubWord(temp);

        for (j = 0; j < 4; j++)
        {
            // printk("temp sau rot va sub : ");
            if (j < 4 && bytesGenerated % 16 < 4)
            {

                temp[j] = temp[j] ^ Rcon[indexRcon];

                indexRcon++;
                expandedKey[bytesGenerated] = temp[j] ^ expandedKey[bytesGenerated - 16];

                bytesGenerated++;
            }
            else
            {

                expandedKey[bytesGenerated] = temp1[j] ^ expandedKey[bytesGenerated - 16];

                bytesGenerated++;
            }
            // printk("\n");
        }
        // printk("line: %d",bytesGenerated);
    }
}

void shiftRows(unsigned char *state)
{
    unsigned char temp;
    unsigned char temp1;
    unsigned char temp2;
    // Dịch hàng thứ hai sang trái 1 byte
    temp = state[4];
    state[4] = state[5];
    state[5] = state[6];
    state[6] = state[7];
    state[7] = temp;

    // Dịch hàng thứ ba sang trái 2 byte
    temp = state[8];
    temp1 = state[9];
    state[8] = state[10];
    state[9] = state[11];
    state[10] = temp;
    state[11] = temp1;

    // Dịch hàng thứ tư sang trái 3 byte
    temp = state[12];
    temp1 = state[13];
    temp2 = state[14];
    state[12] = state[15];
    state[13] = temp;
    state[14] = temp1;
    state[15] = temp2;
}
void invShiftRows(unsigned char *state)
{
    unsigned char temp;
    unsigned char temp1;
    // unsigned char temp2;

    // Dịch hàng thứ hai sang phải 1 byte
    temp = state[7];
    state[7] = state[6];
    state[6] = state[5];
    state[5] = state[4];
    state[4] = temp;

    // Dịch hàng thứ ba sang phải 2 byte
    temp = state[8];
    temp1 = state[9];
    state[8] = state[10];
    state[9] = state[11];
    state[10] = temp;
    state[11] = temp1;

    // Dịch hàng thứ tư sang phải 3 byte
    temp = state[12];
    state[12] = state[13];
    state[13] = state[14];
    state[14] = state[15];
    state[15] = temp;
}

const unsigned char mix_column_matrix[4][4] =
    {
        {0x02, 0x03, 0x01, 0x01},
        {0x01, 0x02, 0x03, 0x01},
        {0x01, 0x01, 0x02, 0x03},
        {0x03, 0x01, 0x01, 0x02}};

const unsigned char inv_mix_column_matrix[4][4] =
    {
        {0x0E, 0x0B, 0x0D, 0x09},
        {0x09, 0x0E, 0x0B, 0x0D},
        {0x0D, 0x09, 0x0E, 0x0B},
        {0x0B, 0x0D, 0x09, 0x0E}};
unsigned char xtime(unsigned char x)
{
    if (x >> 7)
    {
        return ((x << 1) ^ 0x1B);
    }
    else
    {
        return (x << 1);
    }
}
// vi du goi :unsigned char result1 = xtime(0x57);
//
// Hàm MixColumn
void MixColumn(unsigned char *state)
{
    unsigned char tmp[4];
    int i;
    for (i = 0; i < 4; i++)
    {

        tmp[0] = xtime(state[i]) ^ (xtime(state[i + 4]) ^ state[i + 4]) ^ state[i + 8] ^ state[i + 12];
        tmp[1] = state[i] ^ xtime(state[i + 4]) ^ (xtime(state[i + 8]) ^ state[i + 8]) ^ state[i + 12];
        tmp[2] = state[i] ^ state[i + 4] ^ xtime(state[i + 8]) ^ (xtime(state[i + 12]) ^ state[i + 12]);
        tmp[3] = (state[i] ^ xtime(state[i])) ^ state[i + 4] ^ state[i + 8] ^ xtime(state[i + 12]);
        state[i] = tmp[0];
        state[i + 4] = tmp[1];
        state[i + 8] = tmp[2];
        state[i + 12] = tmp[3];
    }
}

unsigned char multiply(unsigned char a, unsigned char b)
{
    unsigned char result = 0;
    unsigned char carry;
    int i;
    for (i = 0; i < 8; ++i)
    {
        if (b & 1)
            result ^= a;
        carry = a & 0x80;
        a <<= 1;
        if (carry)
            a ^= 0x1B; // polynomial x^8 + x^4 + x^3 + x + 1
        b >>= 1;
    }
    return result;
}

// Hàm UnMixColumn
void UnMixColumn(unsigned char *state)
{
    unsigned char tmp[4];
    int i;
    for (i = 0; i < 4; i++)
    {
        tmp[0] = multiply(0x0E, state[i]) ^ multiply(0x0B, state[i + 4]) ^ multiply(0x0D, state[i + 8]) ^ multiply(0x09, state[i + 12]);
        tmp[1] = multiply(0x09, state[i]) ^ multiply(0x0E, state[i + 4]) ^ multiply(0x0B, state[i + 8]) ^ multiply(0x0D, state[i + 12]);
        tmp[2] = multiply(0x0D, state[i]) ^ multiply(0x09, state[i + 4]) ^ multiply(0x0E, state[i + 8]) ^ multiply(0x0B, state[i + 12]);
        tmp[3] = multiply(0x0B, state[i]) ^ multiply(0x0D, state[i + 4]) ^ multiply(0x09, state[i + 8]) ^ multiply(0x0E, state[i + 12]);

        state[i] = tmp[0];
        state[i + 4] = tmp[1];
        state[i + 8] = tmp[2];
        state[i + 12] = tmp[3];
    }
}

void AddRoundKey1(unsigned char *state, unsigned char *roundKey)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        state[i] ^= roundKey[i];
    }
}
void AddRoundKey(unsigned char *state, unsigned char *roundKey)
{
    state[0] ^= roundKey[0];
    state[1] ^= roundKey[4];
    state[2] ^= roundKey[8];
    state[3] ^= roundKey[12];
    state[4] ^= roundKey[1];
    state[5] ^= roundKey[5];
    state[6] ^= roundKey[9];
    state[7] ^= roundKey[13];
    state[8] ^= roundKey[2];
    state[9] ^= roundKey[6];
    state[10] ^= roundKey[10];
    state[11] ^= roundKey[14];
    state[12] ^= roundKey[3];
    state[13] ^= roundKey[7];
    state[14] ^= roundKey[11];
    state[15] ^= roundKey[15];
}
void AddRoundKeyT(unsigned char *state, unsigned char *roundKey)
{
    state[0] = roundKey[0];
    state[1] = roundKey[4];
    state[2] = roundKey[8];
    state[3] = roundKey[12];
    state[4] = roundKey[1];
    state[5] = roundKey[5];
    state[6] = roundKey[9];
    state[7] = roundKey[13];
    state[8] = roundKey[2];
    state[9] = roundKey[6];
    state[10] = roundKey[10];
    state[11] = roundKey[14];
    state[12] = roundKey[3];
    state[13] = roundKey[7];
    state[14] = roundKey[11];
    state[15] = roundKey[15];
}
void xor (unsigned char *state1, unsigned char *state2, unsigned char *result) {
    int i;
    for (i = 0; i < 16; i++)
    {
        result[i] = state1[i] ^ state2[i];
    }
} void AES_Encrypt(unsigned char *state, unsigned char *roundKeys)
{

    AddRoundKey(state, roundKeys);
    int round;
    for (round = 1; round < 10; round++)
    {
        subBytes(state);
        shiftRows(state);
        // printk("Sau khi shiftRow\n");
        MixColumn(state);
        AddRoundKey(state, roundKeys + round * 16);
    }

    subBytes(state);
    shiftRows(state);
    AddRoundKey(state, roundKeys + 10 * 16);
}
void AES_Invecrypt(unsigned char *state1, unsigned char *roundKeys1)
{
    AddRoundKey(state1, roundKeys1 + 10 * 16);

    int round;
    for (round = 1; round < 10; round++)
    {
        invShiftRows(state1);
        invSubBytes(state1);
        AddRoundKey(state1, roundKeys1 + (10 - round) * 16);
        UnMixColumn(state1);
    }
    invShiftRows(state1);
    invSubBytes(state1);
    AddRoundKey(state1, roundKeys1);
}
// end code ma hoa

// Path to output file

void displayState(unsigned char *state)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        printk("%02x ", state[i]);
        if ((i + 1) % 4 == 0)
            printk("\n");
    }
}
// function ma hoa
static unsigned char hex_to_byte(char hex)
{
    if (hex >= '0' && hex <= '9')
    {
        return hex - '0';
    }
    else if (hex >= 'a' && hex <= 'f')
    {
        return hex - 'a' + 10;
    }
    else if (hex >= 'A' && hex <= 'F')
    {
        return hex - 'A' + 10;
    }
    return 0; // Trả về 0 nếu không phải ký tự hexa hợp lệ
}
static void hex_string_to_bytes(const char *hex_string, unsigned char *bytes)
{
    size_t length = strlen(hex_string);
    size_t i;
    for (i = 0; i < length; i += 2)
    {
        bytes[i / 2] = (hex_to_byte(hex_string[i]) << 4) | hex_to_byte(hex_string[i + 1]);
    }
}
void bytesToHexString(unsigned char *bytes, unsigned char *hexString)
{
    size_t i;

    // Convert each byte to hex and store in hex string
    for (i = 0; i < BUF_SIZE; ++i)
    {
        sprintf(hexString + i * 2, "%02X", bytes[i]);
    }

    // Null-terminate the hex string
    hexString[BUF_SIZE * 2] = '\0';
}
void copyHexArray(unsigned char *hexaa, unsigned char *hex_buffer)
{
    int i;
    for (i = 0; i < BUF_SIZE; ++i)
    {
        hex_buffer[i * 2] = (hexaa[i] >> 4) + '0';
        if (hex_buffer[i * 2] > '9')
            hex_buffer[i * 2] += 7; // Chuyển số thành chữ cái A-F
        hex_buffer[i * 2 + 1] = (hexaa[i] & 0xF) + '0';
        if (hex_buffer[i * 2 + 1] > '9')
            hex_buffer[i * 2 + 1] += 7; // Chuyển số thành chữ cái A-F
    }
    hex_buffer[BUF_SIZE * 2] = '\0'; // Đảm bảo kết thúc chuỗi hex_buffer bằng null-terminator
}
// function giai ma
static unsigned char hexToByte(const char *hex)
{
    unsigned char result = 0;
    int i;

    for (i = 0; i < 2; ++i)
    {
        char digit = hex[i];

        if (digit >= '0' && digit <= '9')
        {
            result = (result << 4) | (digit - '0');
        }
        else if (digit >= 'a' && digit <= 'f')
        {
            result = (result << 4) | (digit - 'a' + 10);
        }
        else if (digit >= 'A' && digit <= 'F')
        {
            result = (result << 4) | (digit - 'A' + 10);
        }
        else
        {
            // Xử lý ký tự hexa không hợp lệ (nếu cần thiết)
            printk(KERN_ERR "Invalid hex character '%c'\n", digit);
            return 0; // Hoặc giá trị mặc định khác nếu cần
        }
    }

    return result;
}

// Hàm chuyển đổi mảng hexa chuỗi thành mảng giá trị hexa và cấp phát bộ nhớ cho con trỏ hex_array
static void hexStringToHexArray(const char *hex_string, unsigned char *hex_array)
{
    int i;

    // Chuyển đổi từng cặp ký tự hexa từ hex_string sang hex_array
    for (i = 0; i < 16; ++i)
    {
        hex_array[i] = hexToByte(&hex_string[i * 2]); // Chuyển đổi từng cặp ký tự hexa
    }
}
static void hexPairsToChars(const unsigned char *hex_pairs, char *char_array)
{
    int i;
    static const char hex_to_char[] = "0123456789abcdef"; // Bảng ánh xạ từ hexa sang ký tự

    for (i = 0; i < BUF_SIZE; ++i)
    {
        char_array[i * 2] = hex_to_char[(hex_pairs[i] >> 4) & 0xF]; // Lấy 4 bit cao
        char_array[i * 2 + 1] = hex_to_char[hex_pairs[i] & 0xF];    // Lấy 4 bit thấp
    }
    char_array[32] = '\0'; // Kết thúc chuỗi ký tự
}
static unsigned char hexPairToByte(const char *hex)
{
    unsigned char result = 0;

    // Xử lý ký tự đầu tiên của cặp hexa
    if (hex[0] >= '0' && hex[0] <= '9')
        result = hex[0] - '0';
    else if (hex[0] >= 'a' && hex[0] <= 'f')
        result = hex[0] - 'a' + 10;
    else if (hex[0] >= 'A' && hex[0] <= 'F')
        result = hex[0] - 'A' + 10;

    // Dịch trái 4 bit để chuẩn bị cho ký tự thứ hai
    result = result << 4;

    // Xử lý ký tự thứ hai của cặp hexa
    if (hex[1] >= '0' && hex[1] <= '9')
        result |= hex[1] - '0';
    else if (hex[1] >= 'a' && hex[1] <= 'f')
        result |= hex[1] - 'a' + 10;
    else if (hex[1] >= 'A' && hex[1] <= 'F')
        result |= hex[1] - 'A' + 10;

    return result;
}

// Hàm chuyển đổi mảng buf chứa chuỗi hexa thành chuỗi ký tự
static void hexStringToCharString(const char *buf, char *char_string)
{
    int i, j = 0;
    int len = strlen(buf);

    // Đảm bảo độ dài chuỗi hexa là bội của 2
    if (len % 2 != 0)
    {
        printk(KERN_ALERT "Invalid hex string length\n");
        return;
    }

    // Chuyển đổi từng cặp ký tự hexa thành ký tự tương ứng
    for (i = 0; i < len; i += 2)
    {
        unsigned char byte = hexPairToByte(&buf[i]);
        char_string[j++] = (char)byte;
    }
    char_string[j] = '\0'; // Kết thúc chuỗi ký tự
}
//
//

static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "usb_control: Device opened");
    return 0;
}
// dong character
static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "usb_control: Device closed");
    return 0;
}

// read character device ==> write to ==>  use
static ssize_t device_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int bytes_to_copy;

    // Kiểm tra xem đã đọc xong chưa
    if (device_read_complete)
    {
        return 0; // Trả về 0 để báo hiệu hết dữ liệu
    }

    // Xác định số lượng byte có thể đọc (tối đa là count)
    bytes_to_copy = min(count, (size_t)device_buffer_len);

    // Sao chép dữ liệu từ kernel space (device_buffer) sang user space (buf)
    if (copy_to_user(buf, device_buffer, bytes_to_copy) != 0)
    {
        return -EFAULT; // Lỗi sao chép dữ liệu
    }

    // Đánh dấu đã đọc hoàn thành và reset lại bộ đệm
    device_read_complete = 1;
    device_buffer_len = 0;

    // Cập nhật vị trí con trỏ file và độ dài của bộ đệm
    *ppos = 0;

    return bytes_to_copy;
}
// read use ==> write to ==>  character device
static ssize_t device_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int bytes_to_copy;

    // Xác định số lượng byte có thể ghi (tối đa là count)
    bytes_to_copy = min(count, (size_t)(BUF_SIZE_USB - *ppos));

    // Sao chép dữ liệu từ user space (buf) vào kernel space (device_buffer)
    if (copy_from_user(device_buffer + *ppos, buf, bytes_to_copy) != 0)
    {
        return -EFAULT; // Lỗi sao chép dữ liệu
    }

    // Cập nhật vị trí con trỏ file để đảm bảo lần ghi tiếp theo bắt đầu từ đầu bộ đệm
    *ppos = 0;

    // Cập nhật kích thước thực tế của dữ liệu trong bộ đệm
    device_buffer_len += bytes_to_copy;

    // Đánh dấu chưa đọc hoàn thành khi có dữ liệu mới ghi vào
    device_read_complete = 0;

    return bytes_to_copy;
}

// ioctl character device
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    int err = 0;
    if (copy_from_user(file_name, (char *)arg, MAX_FILENAME_LEN))
    {
        err = -EFAULT;
    }
    printk(KERN_INFO "usb_crypto: Received filename from user-space: %s\n", file_name);
    // Điều khiển thiết bị (tạm thời không thực hiện gì)
    printk(KERN_INFO "device_ioctl start\n");
    printk(KERN_INFO "device_ioctl received %u\n", cmd);
    printk(KERN_INFO "device_ioctl: received cmd = %u\n", cmd);
    printk(KERN_INFO "Expected IOCTL_ENCRYPT = %u\n", IOCTL_ENCRYPT);
    printk(KERN_INFO "Expected IOCTL_DECRYPT = %u\n", IOCTL_DECRYPT);
    // Điều khiển thiết bị (tạm thời không thực hiện gì)
    switch (cmd)
    {
    case IOCTL_ENCRYPT:
        printk(KERN_INFO "MA hoa data\n");
        struct file *input_file, *output_file;
        char *buf;
        unsigned char *hex_buffer;
        unsigned char hex_buffer_after[BUF_SIZE * 2 + 1];
        mm_segment_t old_fs;
        int bytes_read, bytes_written;
        unsigned char *bytes;
        printk(KERN_INFO "File IO module loaded\n");
        bytes = kmalloc(16, GFP_KERNEL);
        if (!bytes)
        {
            printk(KERN_ERR "Failed to allocate memory\n");
            return -ENOMEM;
        }
        buf = kmalloc(BUF_SIZE + 1, GFP_KERNEL);
        if (!buf)
        {
            printk(KERN_ALERT "Failed to allocate memory for buf\n");
            return -ENOMEM;
        }

        hex_buffer = kmalloc(BUF_SIZE * 2 + 1, GFP_KERNEL);
        if (!hex_buffer)
        {
            printk(KERN_ALERT "Failed to allocate memory for hex_buffer\n");
            kfree(buf);
            return -ENOMEM;
        }
        // Open input file
        input_file = filp_open(file_name, O_RDONLY, 0);
        if (IS_ERR(input_file))
        {
            printk(KERN_ALERT "Failed to open input file\n");
            return PTR_ERR(input_file);
        }

        // Open output file
        output_file = filp_open(file_b, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (IS_ERR(output_file))
        {
            printk(KERN_ALERT "Failed to open output file\n");
            filp_close(input_file, NULL);
            return PTR_ERR(output_file);
        }

        // Read from input file and write to output file
        old_fs = get_fs();
        set_fs(KERNEL_DS);

        while ((bytes_read = vfs_read(input_file, buf, BUF_SIZE, &input_file->f_pos)) > 0)
        {
            if (bytes_read < BUF_SIZE)
            {
                // Điền khoảng trống vào chỗ còn lại trong bộ đệm buf
                memset(buf + bytes_read - 1, 0x20, BUF_SIZE - bytes_read + 1);
            }

            buf[BUF_SIZE] = '\0'; // Đảm bảo bộ đệm buf kết thúc bằng null-terminator

            printk(KERN_INFO "Read chunk: %s\n", buf);
            int i;
            for (i = 0; i < BUF_SIZE; ++i)
            {
                hex_buffer[i * 2] = (buf[i] >> 4) + '0';
                if (hex_buffer[i * 2] > '9')
                    hex_buffer[i * 2] += 7; // Chuyển số thành chữ cái A-F
                hex_buffer[i * 2 + 1] = (buf[i] & 0xF) + '0';
                if (hex_buffer[i * 2 + 1] > '9')
                    hex_buffer[i * 2 + 1] += 7; // Chuyển số thành chữ cái A-F
            }
            hex_buffer[2 * BUF_SIZE] = '\0'; // Đảm bảo chuỗi kết thúc bằng null-terminator
            // hex_buffer đang là con trỏ chuỗi
            printk(KERN_INFO "Hex values: %s\n", hex_buffer);
            // bytes_written = vfs_write(output_file, hex_buffer_after, 2 * BUF_SIZE, &output_file->f_pos);
            hex_string_to_bytes(hex_buffer, bytes);
            //   bytes là con trỏ mảng
            displayState(bytes);
            AES_Encrypt(bytes, expandedKey);
            printk(KERN_INFO "Du lieu sau khi ma hoa\n");
            displayState(bytes);
            copyHexArray(bytes, hex_buffer_after);
            AES_Invecrypt(bytes, expandedKey);
            printk(KERN_INFO "Du lieu sau khi giai ma hoa\n");
            displayState(bytes);
            //   bytes_written = vfs_write(output_file, hex_buffer, 2 * BUF_SIZE, &output_file->f_pos);
            printk(KERN_INFO "Hex values after ma hoa: %s\n", hex_buffer_after);
            bytes_written = vfs_write(output_file, hex_buffer_after, 2 * BUF_SIZE, &output_file->f_pos);
            if (bytes_written < 0)
            {
                printk(KERN_ALERT "Failed to write to output file\n");
                break;
            }
        }
        set_fs(old_fs);
        kfree(bytes);
        kfree(hex_buffer);
        kfree(buf);

        // Đóng tệp
        filp_close(input_file, NULL);
        filp_close(output_file, NULL);
        break;
    case IOCTL_DECRYPT:
        // Thực hiện giải mã
        printk(KERN_INFO "Gia Ma data\n");
        char buf1[BUF_SIZE * 2 + 1];
        char char_array[BUF_SIZE * 2 + 1];
        // char byte_buffer[BUF_SIZE * 2 + 1];
        char char_string[BUF_SIZE + 1];
        mm_segment_t old_fs1;
        int bytes_read1, bytes_written1;
        unsigned char *hexArray;
        struct file *inputcipher_file, *outputcipher_file;
        printk(KERN_INFO "File IO module loaded\n");
        // open input file
        inputcipher_file = filp_open(file_name, O_RDONLY, 0);
        if (IS_ERR(inputcipher_file))
        {
            printk(KERN_ALERT "Failed to open input file\n");
            return PTR_ERR(inputcipher_file);
        }

        // Open output file
        outputcipher_file = filp_open(file_c, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (IS_ERR(outputcipher_file))
        {
            printk(KERN_ALERT "Failed to open output file\n");
            filp_close(inputcipher_file, NULL);
            return PTR_ERR(outputcipher_file);
        }
        hexArray = kmalloc(BUF_SIZE + 1, GFP_KERNEL);
        if (!hexArray)
        {
            printk(KERN_ALERT "Failed to allocate memory\n");
            filp_close(inputcipher_file, NULL);
            filp_close(outputcipher_file, NULL);
            return -ENOMEM;
        }
        old_fs1 = get_fs();
        set_fs(KERNEL_DS);

        while ((bytes_read1 = vfs_read(inputcipher_file, buf1, BUF_SIZE * 2, &inputcipher_file->f_pos)) > 0)
        {
            if (bytes_read1 % 2 == 1)
            {
                break;
            }
            buf1[BUF_SIZE * 2] = '\0'; // Đảm bảo bộ đệm buf kết thúc bằng null-terminator
            printk(KERN_INFO "Read chunk: %s\n", buf1);
            hexStringToHexArray(buf1, hexArray);
            hexArray[BUF_SIZE] = '\0';
            printk(KERN_INFO "Chuyen thanh mang buffer\n");
            // displayState(byte_buffer);
            AES_Invecrypt(hexArray, expandedKey);
            hexPairsToChars(hexArray, char_array); // loi o day
            hexStringToCharString(char_array, char_string);
            printk(KERN_INFO "Du lieu sau khi giai ma hoa\n");
            displayState(hexArray);
            printk(KERN_INFO "Chuyen mang da giai ma thanh chuoi: %s\n", char_string);
            bytes_written1 = vfs_write(outputcipher_file, char_string, BUF_SIZE, &outputcipher_file->f_pos);
            if (bytes_written1 < 0)
            {
                printk(KERN_ALERT "Failed to write to output file\n");
                break;
            }
        }

        set_fs(old_fs1);

        // Giải phóng bộ nhớ đã cấp phát
        kfree(hexArray);

        // Đóng tệp
        filp_close(inputcipher_file, NULL);
        filp_close(outputcipher_file, NULL);
        break;
    default:
        printk(KERN_INFO "device_ioctl: Invalid command\n");
        return -EINVAL; // Lệnh không hợp lệ
    }
    printk(KERN_INFO "device_ioctl end");
    return retval;
}
// file discriptions của character device
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
};
// usb_device connected, char_device is connected
static int simple_usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    int result;
    crypto_usb_device = interface_to_usbdev(interface);
    if (!crypto_usb_device)
    {
        printk(KERN_ERR "Failed to obtain USB device\n");
        return -ENODEV;
    }
    printk(KERN_INFO "Simple USB device (%04X:%04X) plugged\n", id->idVendor, id->idProduct);

    crypto_device_number = MKDEV(MY_MAJOR, MY_MINOR); // Sử dụng major = 42, minor = 0
    result = register_chrdev_region(crypto_device_number, NUM_MINORS, DEVICE_NAME);
    if (result < 0)
    {
        printk(KERN_ERR "Failed to register char device region\n");
        return result;
    }

    class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(class))
    {
        unregister_chrdev_region(crypto_device_number, 1);
        printk(KERN_ERR "Failed to create the device class\n");
        return PTR_ERR(class);
    }

    if (device_create(class, NULL, crypto_device_number, NULL, DEVICE_NAME) == NULL)
    {
        class_destroy(class);
        unregister_chrdev_region(crypto_device_number, 1);
        printk(KERN_ERR "Failed to create the device\n");
        return -1;
    }

    cdev_init(&cdev, &fops);
    cdev.owner = THIS_MODULE;
    if (cdev_add(&cdev, crypto_device_number, 1) == -1)
    {
        device_destroy(class, crypto_device_number);
        class_destroy(class);
        unregister_chrdev_region(crypto_device_number, 1);
        printk(KERN_ERR "Failed to add cdev\n");
        return -1;
    }
    printk(KERN_INFO "Simple USB device (%04X:%04X) attached\n", id->idVendor, id->idProduct);
    ExpandKey(key, expandedKey);
    return 0;
}
// usb_device connected, char_device is connected
static void simple_usb_disconnect(struct usb_interface *interface)
{
    struct usb_device *usb_dev = interface_to_usbdev(interface);
    printk(KERN_INFO "Simple USB device removed\n");
    cdev_del(&cdev);
    device_destroy(class, crypto_device_number);
    class_destroy(class);
    unregister_chrdev_region(crypto_device_number, 1);
    printk(KERN_INFO "Simple USB device (%04X:%04X) disconnected\n",
           usb_dev->descriptor.idVendor, usb_dev->descriptor.idProduct);
}

// Define table usb device is supported
static struct usb_device_id simple_usb_table[] = {
    {USB_DEVICE(VENDOR_ID, PRODUCT_ID)},
    {} // Dấu kết thúc bảng
};
MODULE_DEVICE_TABLE(usb, simple_usb_table);
// Implement usb_device
static struct usb_driver simple_usb_driver = {
    .name = "usb_driver",
    .id_table = simple_usb_table,
    .probe = simple_usb_probe,           // usb_device connected
    .disconnect = simple_usb_disconnect, // usb_device disconnected
};
// Declare usb_device
static int __init simple_usb_init(void)
{
    int result;

    result = usb_register(&simple_usb_driver);
    if (result == 0)
        printk(KERN_INFO "USB driver crypto registered\n");
    else
        printk(KERN_ERR "Failed to register Simple USB driver: %d\n", result);

    return result;
}
// Remove usb_device
static void __exit simple_usb_exit(void)
{
    usb_deregister(&simple_usb_driver);
    printk(KERN_INFO "Simple USB driver unregistered\n");
}
module_init(simple_usb_init);
module_exit(simple_usb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("USB Driver Crypto");

