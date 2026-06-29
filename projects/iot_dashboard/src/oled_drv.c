/**
 * OLED SSD1306 128x64 像素驱动
 * 基于 sysfs GPIO 软件模拟 SPI，从 app_code/oled_spi_app.c 移植并模块化
 */

#include <stdio.h>
#include "oled_drv.h"

/* TODO: 移植 GPIO SPI 底层操作、字库、显示接口 */

int oled_init(void) {
    return 0;
}

void oled_clear(void) {
}

void oled_fill(unsigned char data) {
    (void)data;
}

void oled_show_string(int x, int page, const char *str) {
    (void)x; (void)page; (void)str;
}

void oled_show_num(int x, int page, int num) {
    (void)x; (void)page; (void)num;
}

void oled_refresh(void) {
}
