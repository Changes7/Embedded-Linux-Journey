#ifndef OLED_DRV_H
#define OLED_DRV_H

int oled_init(void);
void oled_clear(void);
void oled_fill(unsigned char data);
void oled_show_string(int x, int page, const char *str);
void oled_show_num(int x, int page, int num);
void oled_refresh(void);

#endif
