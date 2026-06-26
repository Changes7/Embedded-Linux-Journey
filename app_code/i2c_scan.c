#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int main(int argc, char *argv[]) {
    int fd;
    if (argc < 2) {
        printf("用法: sudo %s <I2C设备节点>\n", argv[0]);
        printf("示例: sudo %s /dev/i2c-5\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("[错误] 无法打开 I2C 节点，请检查通道号或使用 sudo");
        return -1;
    }

    printf("\n正在扫描 I2C 总线: %s\n", argv[1]);
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            int addr = i + j;
            // I2C 协议保留地址不扫描
            if (addr < 0x03 || addr > 0x77) {
                printf("   ");
                continue;
            }

            // 尝试将通信目标设置为该地址
            if (ioctl(fd, I2C_SLAVE, addr) < 0) {
                printf("-- ");
                continue;
            }

            // 尝试写入一个字节的哑数据 (0x00 对于 SSD1306 OLED 非常安全且兼容)
            unsigned char test_byte = 0x00;
            if (write(fd, &test_byte, 1) == 1) {
                printf("%02x ", addr); // 响应成功，打印出物理地址！
            } else {
                printf("-- ");         // 无响应
            }
        }
        printf("\n");
    }

    close(fd);
    return 0;
}