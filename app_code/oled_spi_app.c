#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// 绑定映射计算出的 GPIO 物理编号
#define GPIO_CLK  "161" // PF1 -> 时钟
#define GPIO_SDA  "160" // PF0 -> 数据 (MOSI)
#define GPIO_RES  "165" // PF5 -> 复位
#define GPIO_DC   "164" // PF4 -> 数据/命令切换
#define GPIO_CS   "162" // PF2 -> 片选

// GPIO 底层控制辅助函数
int gpio_export(const char *pin) {
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) return -1;
    write(fd, pin, strlen(pin));
    close(fd);
    return 0;
}

int gpio_direction(const char *pin, const char *dir) {
    char path[64];
    sprintf(path, "/sys/class/gpio/gpio%s/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    write(fd, dir, strlen(dir));
    close(fd);
    return 0;
}

int gpio_write(const char *pin, int val) {
    char path[64];
    sprintf(path, "/sys/class/gpio/gpio%s/value", pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    write(fd, val ? "1" : "0", 1);
    close(fd);
    return 0;
}

// 软件模拟 4线 SPI 发送单字节
void spi_send_byte(unsigned char data) {
    for (int i = 0; i < 8; i++) {
        gpio_write(GPIO_CLK, 0); // 拉低时钟，准备数据
        gpio_write(GPIO_SDA, (data & 0x80) ? 1 : 0); // 写入最高位
        gpio_write(GPIO_CLK, 1); // 拉高时钟，告知屏幕读取数据
        data <<= 1;
    }
}

// 写命令与写数据封装
void oled_write_cmd(unsigned char cmd) {
    gpio_write(GPIO_DC, 0); // DC = 0，代表命令
    gpio_write(GPIO_CS, 0); // 选中屏幕
    spi_send_byte(cmd);
    gpio_write(GPIO_CS, 1); // 释放屏幕
}

void oled_write_data(unsigned char data) {
    gpio_write(GPIO_DC, 1); // DC = 1，代表数据
    gpio_write(GPIO_CS, 0); // 选中屏幕
    spi_send_byte(data);
    gpio_write(GPIO_CS, 1); // 释放屏幕
}

// OLED 初始化序列
void oled_init(void) {
    // 硬件复位屏幕
    gpio_write(GPIO_RES, 1);
    usleep(100000);
    gpio_write(GPIO_RES, 0);
    usleep(200000);
    gpio_write(GPIO_RES, 1);
    usleep(100000);

    oled_write_cmd(0xAE); // 关闭显示
    oled_write_cmd(0x20); // 设置内存寻址模式
    oled_write_cmd(0x10); // 行寻址模式
    oled_write_cmd(0xB0); // 设置页地址
    oled_write_cmd(0xC8); // 翻转行扫描
    oled_write_cmd(0x00); // 设置低列地址
    oled_write_cmd(0x10); // 设置高列地址
    oled_write_cmd(0x40); // 开始行地址
    oled_write_cmd(0x81); // 对比度设置
    oled_write_cmd(0xFF); // 亮度最大
    oled_write_cmd(0xA1); // 翻转列扫描
    oled_write_cmd(0xA6); // 正常显示
    oled_write_cmd(0xA8); // 设置多路复用率
    oled_write_cmd(0x3F);
    oled_write_cmd(0xA4); // 全屏点亮输出
    oled_write_cmd(0xD3); // 设置显示偏移
    oled_write_cmd(0x00);
    oled_write_cmd(0xD5); // 时钟分频
    oled_write_cmd(0xF0);
    oled_write_cmd(0xD9); // 预充电
    oled_write_cmd(0x22);
    oled_write_cmd(0xDA); // COM 引脚硬件配置
    oled_write_cmd(0x12);
    oled_write_cmd(0xDB); // VCOMH
    oled_write_cmd(0x20);
    oled_write_cmd(0x8D); // 充电泵设置
    oled_write_cmd(0x14); // 开启充电泵
    oled_write_cmd(0xAF); // 开启显示
}

// 全屏填充与清屏
void oled_clear(unsigned char fill_data) {
    for (unsigned char i = 0; i < 8; i++) {
        oled_write_cmd(0xB0 + i); // 设置页地址 (0-7)
        oled_write_cmd(0x00);     // 设置列低地址
        oled_write_cmd(0x10);     // 设置列高地址
        for (unsigned char n = 0; n < 128; n++) {
            oled_write_data(fill_data);
        }
    }
}

int main(void) {
    printf("======================================\n");
    printf("[OLED] 正在导出并初始化 GPIO 引脚...\n");
    printf("======================================\n");

    // 1. 导出所有的控制引脚
    gpio_export(GPIO_CLK);
    gpio_export(GPIO_SDA);
    gpio_export(GPIO_RES);
    gpio_export(GPIO_DC);
    gpio_export(GPIO_CS);

    // 等待系统文件系统反应时间
    usleep(100000);

    // 2. 将引脚全部方向配置为“输出 (out)”
    gpio_direction(GPIO_CLK, "out");
    gpio_direction(GPIO_SDA, "out");
    gpio_direction(GPIO_RES, "out");
    gpio_direction(GPIO_DC, "out");
    gpio_direction(GPIO_CS, "out");

    // 3. 硬件点火
    oled_init();
    
    printf("[OLED] 开始考机点亮 3 秒...\n");
    oled_clear(0xFF); // 0xFF 代表所有像素点全亮
    sleep(3);

    printf("[OLED] 考机完毕，进行清屏...\n");
    oled_clear(0x00); // 0x00 清屏

    return 0;
}