#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// 根据你的开发板实际情况，定义LED亮度文件的路径
#define LED_BRIGHTNESS_PATH "/sys/class/leds/green/brightness"

int main(int argc, char *argv[]) {
    int fd;
    
    printf("======================================\n");
    printf("  STM32MP157 LED Sysfs Control Demo   \n");
    printf("======================================\n");

    // 1. 使用标准文件系统系统调用 open() 打开这个硬件映射文件
    // O_WRONLY 表示只写方式打开
    fd = open(LED_BRIGHTNESS_PATH, O_WRONLY);
    if (fd < 0) {
        perror("[ERROR] 无法打开LED文件，请检查路径或使用 sudo 运行");
        return EXIT_FAILURE;
    }

    printf("[INFO] 成功打开LED控制接口，开始闪烁测试...\n");

    // 2. 循环闪烁 5 次
    for (int i = 0; i < 5; i++) {
        printf("[正常闪烁] 第 %d 次：点亮 LED\n", i + 1);
        // 万物皆文件：向文件写入字符串 "1" 即可触发硬件电平翻转
        write(fd, "1", 1); 
        sleep(1); // 延时 1 秒

        printf("[正常闪烁] 第 %d 次：熄灭 LED\n", i + 1);
        // 向文件写入字符串 "0" 熄灭 LED
        write(fd, "0", 1); 
        sleep(1); // 延时 1 秒
    }

    // 3. 规范编写：关闭文件描述符
    close(fd);
    printf("[INFO] 测试结束，释放文件资源。\n");
    
    return EXIT_SUCCESS;
}