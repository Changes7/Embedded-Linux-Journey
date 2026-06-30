# STM32MP157 IoT Dashboard 项目复盘

本项目位于：

```text
Embedded-Linux-Journey/projects/iot_dashboard/
```

项目目标是基于 STM32MP157 嵌入式 Linux 开发板实现一个物联网仪表盘，逐步覆盖 OLED 显示、JSON 解析、MQTT 通信、多线程状态同步、GPIO 控制等嵌入式 Linux 求职常见技能点。

---

## 1. 今天完成的两步

| 阶段 | 内容 | 当前状态 |
|---|---|---|
| 阶段 1 | OLED 驱动模块化：GPIO 模拟 SPI，SSD1306 初始化，显示字符串 | 已完成 |
| 阶段 2 | 集成 cJSON：JSON 字符串解析为状态结构体，并显示到 OLED | 已完成 |

当前已经打通链路：

```text
模拟 MQTT JSON 字符串
    ↓
cJSON 解析
    ↓
dashboard_state_t 状态结构体
    ↓
OLED 显示
```

---

## 2. 当前工程结构

```text
iot_dashboard/
├── Makefile
├── include/
│   ├── dashboard_state.h
│   ├── json_parser.h
│   ├── mqtt_client.h
│   └── oled_drv.h
├── src/
│   ├── dashboard_state.c
│   ├── json_parser.c
│   ├── main.c
│   ├── mqtt_client.c
│   └── oled_drv.c
└── third_party/
    ├── cJSON/
    │   ├── cJSON.c
    │   └── cJSON.h
    └── paho.mqtt.c/
```

目录职责：

| 目录 | 说明 |
|---|---|
| `include/` | 对外头文件，定义模块接口 |
| `src/` | 项目核心源码 |
| `third_party/cJSON/` | 第三方 JSON 解析库 |
| `third_party/paho.mqtt.c/` | 后续 MQTT 库目录 |
| `build/` | Makefile 自动生成的编译产物目录 |

---

## 3. 阶段 1：OLED 驱动模块化

### 3.1 本阶段目标

将之前单文件 OLED 测试程序，整理成可复用驱动模块：

```text
include/oled_drv.h
src/oled_drv.c
```

主程序不再直接操作 GPIO，而是通过接口调用：

```c
oled_init();
oled_show_string(0, 0, "STM32MP157 IoT");
oled_refresh();
```

这样做的意义：

- 底层硬件控制和上层业务逻辑解耦。
- 后续 MQTT、JSON、UI 刷新都可以直接调用 OLED 模块。
- 工程结构更接近真实嵌入式项目。

---

### 3.2 OLED 硬件接线

OLED 是 0.96 寸 7 针 SSD1306 屏幕，引脚顺序：

```text
GND VDD SCK SDA RES DC CS
```

实际接线：

| OLED 引脚 | 开发板引脚 | GPIO 编号 |
|---|---|---|
| GND | GND | - |
| VDD | 3V3 | - |
| SCK | SPI_CLK | GPIO90 / PF10 |
| SDA | SPI_IO0 | GPIO88 / PF8 |
| RES | SPI_IO2 | GPIO87 / PF7 |
| DC | SPI_IO1 | GPIO89 / PF9 |
| CS | SPI_NCS | GPIO22 / PB6 |

重点：

- `GPIOF base = 80`，所以 `PF10 = 90`，`PF8 = 88`，`PF9 = 89`，`PF7 = 87`。
- `SPI_NCS` 最终确认是 `PB6`，所以 `GPIOB base = 16`，`PB6 = 22`。
- VDD 接 3V3，不要接 5V。

---

### 3.3 `oled_drv.h` 接口

```c
int oled_init(void);
void oled_clear(void);
void oled_fill(unsigned char data);
void oled_show_string(int x, int page, const char *str);
void oled_show_num(int x, int page, int num);
void oled_refresh(void);
```

| 函数 | 作用 |
|---|---|
| `oled_init()` | 初始化 GPIO、复位 OLED、发送 SSD1306 初始化命令 |
| `oled_clear()` | 清空显存并刷新屏幕 |
| `oled_fill(data)` | 用指定数据填充整屏，`0xFF` 全亮，`0x00` 全灭 |
| `oled_show_string()` | 把字符串写入显存 |
| `oled_show_num()` | 将数字转成字符串后显示 |
| `oled_refresh()` | 把显存真正发送到 OLED 屏幕 |

---

### 3.4 Linux sysfs GPIO 基础

用户态通过 `/sys/class/gpio` 操作 GPIO：

```text
/sys/class/gpio/export
/sys/class/gpio/gpioXX/direction
/sys/class/gpio/gpioXX/value
```

基本流程：

```text
向 export 写 GPIO 编号
    ↓
设置 direction 为 out
    ↓
向 value 写 0 或 1
```

项目中的三个底层函数：

```c
static int gpio_export(const char *pin);
static int gpio_direction(const char *pin, const char *dir);
static int gpio_write(const char *pin, int val);
```

它们本质上都是 Linux 文件 IO：

- `open()` 打开 sysfs 文件
- `write()` 写入控制值
- `close()` 关闭文件描述符

这体现了 Linux 中“一切皆文件”的思想。

---

### 3.5 软件模拟 SPI

当前没有使用 Linux SPI 子系统，而是通过 GPIO 手动模拟 SPI 时序。

模式：

```text
CPOL = 0
CPHA = 0
MSB first
```

发送一个字节的核心逻辑：

```c
for (int i = 7; i >= 0; i--) {
    gpio_write(GPIO_CLK, 0);
    gpio_write(GPIO_SDA, (data >> i) & 0x01);
    gpio_write(GPIO_CLK, 1);
}
gpio_write(GPIO_CLK, 0);
```

理解：

- `SDA` 是数据线。
- `CLK` 是时钟线。
- 每次时钟上升沿，OLED 读取当前数据位。
- `i` 从 7 到 0，说明先发最高位，叫 MSB first。

---

### 3.6 命令和数据

SSD1306 使用 `DC` 引脚区分命令和数据：

| DC | 含义 |
|---|---|
| 0 | 命令 |
| 1 | 数据 |

封装函数：

```c
static void oled_write_cmd(unsigned char cmd);
static void oled_write_data(unsigned char data);
```

`CS` 是片选：

- `CS = 0`：选中 OLED
- `CS = 1`：释放 OLED

所以发送一次命令的流程是：

```text
DC = 0
CS = 0
发送 1 字节命令
CS = 1
```

---

### 3.7 OLED 显存模型

SSD1306 分辨率是：

```text
128 x 64
```

每 8 行组成 1 页，所以：

```text
64 / 8 = 8 页
```

每页 128 列，所以总显存：

```text
128 * 8 = 1024 字节
```

项目中定义：

```c
static unsigned char g_vram[128 * 8];
```

寻址方式：

```c
g_vram[page * 128 + x]
```

其中：

- `page` 范围：0~7
- `x` 范围：0~127

`oled_show_string()` 只是把字符点阵写入 `g_vram`，真正显示到屏幕需要再调用：

```c
oled_refresh();
```

---

## 4. 阶段 2：cJSON 解析与状态显示

### 4.1 本阶段目标

模拟云端通过 MQTT 下发 JSON：

```json
{"temperature":26.5,"humidity":58,"led":1,"message":"Hello IoT"}
```

完成处理链路：

```text
JSON 字符串
    ↓
cJSON_Parse()
    ↓
dashboard_state_t
    ↓
OLED 显示
```

---

### 4.2 共享状态结构体

定义在 `include/dashboard_state.h`：

```c
typedef struct {
    float temperature;
    float humidity;
    int led_state;
    char message[64];
    pthread_mutex_t lock;
} dashboard_state_t;
```

字段说明：

| 字段 | 作用 |
|---|---|
| `temperature` | 温度 |
| `humidity` | 湿度 |
| `led_state` | LED 状态，0 为 OFF，1 为 ON |
| `message` | 云端下发的文本消息 |
| `lock` | 互斥锁，保护多线程共享数据 |

为什么现在就加锁？

后续项目会变成：

```text
MQTT 线程写 state
OLED 线程读 state
```

如果没有互斥锁，可能发生数据竞争。比如 OLED 正在读 `message`，MQTT 线程同时更新 `message`，就可能显示错误或不完整内容。

---

### 4.3 状态初始化和销毁

`src/dashboard_state.c` 提供：

```c
int dashboard_state_init(dashboard_state_t *state);
void dashboard_state_destroy(dashboard_state_t *state);
```

初始化流程：

```text
检查指针
    ↓
memset 清零
    ↓
设置默认值
    ↓
pthread_mutex_init 初始化锁
```

重要函数：

| 函数 | 说明 |
|---|---|
| `memset()` | 清零结构体 |
| `snprintf()` | 安全写字符串 |
| `pthread_mutex_init()` | 初始化互斥锁 |
| `pthread_mutex_destroy()` | 销毁互斥锁 |

---

### 4.4 JSON 解析接口

定义在 `include/json_parser.h`：

```c
int json_parse_control(const char *payload, dashboard_state_t *state);
int json_build_status(const dashboard_state_t *state, char *buf, int buf_size);
```

| 函数 | 作用 |
|---|---|
| `json_parse_control()` | 解析下发 JSON，更新状态结构体 |
| `json_build_status()` | 将当前状态重新封装为 JSON 字符串 |

---

### 4.5 `json_parse_control()` 流程

核心步骤：

```text
检查 payload 和 state
    ↓
cJSON_Parse(payload)
    ↓
解析失败则打印 cJSON_GetErrorPtr()
    ↓
pthread_mutex_lock()
    ↓
读取 temperature / humidity / led / message
    ↓
更新 dashboard_state_t
    ↓
pthread_mutex_unlock()
    ↓
cJSON_Delete(root)
```

重要 cJSON 函数：

| 函数 | 作用 |
|---|---|
| `cJSON_Parse()` | 把 JSON 字符串解析成树 |
| `cJSON_GetObjectItemCaseSensitive()` | 按字段名查找节点 |
| `cJSON_IsNumber()` | 判断是否数字 |
| `cJSON_IsString()` | 判断是否字符串 |
| `cJSON_Delete()` | 释放 JSON 树 |
| `cJSON_GetErrorPtr()` | 获取解析错误位置 |

注意：

`cJSON_Parse()` 会动态申请内存，用完必须 `cJSON_Delete()`，否则长期运行会内存泄漏。

---

### 4.6 `json_build_status()` 流程

该函数把状态结构体重新打包为 JSON，用于后续 MQTT 上报。

流程：

```text
加锁读取 state
    ↓
cJSON_CreateObject()
    ↓
添加 temperature / humidity / led / message
    ↓
cJSON_PrintUnformatted()
    ↓
snprintf() 拷贝到外部 buf
    ↓
cJSON_free(json_str)
    ↓
cJSON_Delete(root)
    ↓
解锁
```

为什么用 `cJSON_PrintUnformatted()`？

- 输出没有空格和换行。
- 数据更短。
- 适合 MQTT payload 网络传输。

---

## 5. 当前 `main.c` 执行流程

当前主程序是阶段 2 的端到端测试。

流程：

```text
启动程序
    ↓
初始化 dashboard_state_t
    ↓
解析模拟 JSON payload
    ↓
打印解析结果
    ↓
将状态重新序列化为 JSON
    ↓
初始化 OLED
    ↓
显示温度、湿度、LED 状态、消息
    ↓
保持 5 秒
    ↓
清屏并退出
```

关键模拟 payload：

```c
static const char *test_payload =
    "{\"temperature\":26.5,\"humidity\":58,\"led\":1,\"message\":\"Hello IoT\"}";
```

C 字符串里 JSON 的双引号需要写成 `\"`，因为双引号本身是 C 字符串边界。

---

## 6. Makefile 复盘

### 6.1 工具链

```makefile
CC = /opt/toolchain/.../arm-none-linux-gnueabihf-gcc
```

这是交叉编译器：

- 在 Ubuntu 主机运行。
- 生成 ARM Linux 程序。
- 程序在 STM32MP157 开发板运行。

### 6.2 编译参数

```makefile
CFLAGS = -Wall -Wextra -O2 -I./include -I./third_party/cJSON -MMD -MP
```

| 参数 | 作用 |
|---|---|
| `-Wall` | 开启常见警告 |
| `-Wextra` | 开启更多警告 |
| `-O2` | 二级优化 |
| `-I./include` | 添加项目头文件路径 |
| `-I./third_party/cJSON` | 添加 cJSON 头文件路径 |
| `-MMD -MP` | 自动生成头文件依赖 |

项目要求：尽量保持 **零警告编译**。

### 6.3 自动收集源文件

```makefile
SRCS  = $(wildcard $(SRC_DIR)/*.c)
SRCS += $(THIRD_SRCS)
```

这样新增 `.c` 文件后，Makefile 通常不需要手动修改。

### 6.4 编译和运行

Ubuntu 主机：

```bash
cd ~/Desktop/Embedded-Linux-Journey/projects/iot_dashboard
make clean
make
cp build/iot_dashboard ~/nfs_rootfs/
```

开发板：

```bash
sudo /mnt/iot_dashboard
```

---

## 7. 当前运行效果

终端预期输出：

```text
[dashboard] 阶段 2: JSON → OLED 端到端测试
[dashboard] 解析 payload: {"temperature":26.5,"humidity":58,"led":1,"message":"Hello IoT"}
[dashboard] 解析结果 -> temp=26.5  humi=58  led=1  msg=Hello IoT
[dashboard] 上报 JSON: {"temperature":26.5,"humidity":58,"led":1,"message":"Hello IoT"}
[dashboard] OLED 刷新完成，保持 5 秒
[dashboard] 测试完成
```

OLED 预期显示：

```text
STM32MP157 IoT

Temp: 26.5 C
Humi: 58 %
LED : ON

Hello IoT
```

---

## 8. 今天必须掌握的基础知识

### C 语言

| 知识点 | 项目体现 |
|---|---|
| 指针 | `dashboard_state_t *state` |
| 字符串 | JSON payload、OLED 显示字符串 |
| 数组 | `g_vram[1024]`、`message[64]` |
| 结构体 | `dashboard_state_t` |
| `static` | 限制函数和全局变量作用域 |
| `const char *` | 只读字符串 payload |
| `snprintf()` | 安全格式化字符串 |
| 返回值检查 | 初始化或解析失败时返回错误 |

### Linux 应用编程

| 知识点 | 项目体现 |
|---|---|
| `open()` | 打开 sysfs GPIO 文件 |
| `write()` | 写 GPIO 电平 |
| `close()` | 关闭文件描述符 |
| `usleep()` | OLED 复位和 GPIO 稳定延时 |
| `sleep()` | 测试显示保持 5 秒 |
| `stderr` | 输出错误日志 |

### 嵌入式硬件通信

| 知识点 | 项目体现 |
|---|---|
| GPIO | 控制 OLED 引脚高低电平 |
| SPI | 通过 CLK/SDA 模拟 SPI |
| CS | 片选 OLED |
| DC | 区分命令和数据 |
| RES | 硬件复位 OLED |
| SSD1306 | OLED 控制器初始化和显存刷新 |

### 多线程基础

当前虽然还没启动线程，但已经在状态结构体中加入：

```c
pthread_mutex_t lock;
```

需要理解：

- 共享数据需要互斥锁保护。
- `pthread_mutex_lock()` 和 `pthread_mutex_unlock()` 必须成对出现。
- 后续 MQTT 线程写状态，OLED 线程读状态。

### JSON 和物联网

MQTT 收到的 payload 本质是字符串。JSON 是这段字符串的业务格式。

后续 MQTT 消息会直接交给：

```c
json_parse_control(payload, &state);
```

所以阶段 2 已经提前完成 MQTT 消息处理的核心业务层。

---

## 9. 常见问题复盘

### OLED 不亮排查

1. VDD 是否接 3V3。
2. GND 是否接 GND。
3. SCK/SDA 是否接反。
4. RES/DC 是否接反。
5. CS 是否接到真正的 SPI_NCS。
6. GPIO 编号是否正确。
7. 程序是否使用 `sudo` 运行。

本项目最终确认：

```text
SPI_NCS = PB6 = GPIO22
```

### 为什么 `oled_show_string()` 后屏幕没变？

因为它只写显存 `g_vram`，真正刷新屏幕需要：

```c
oled_refresh();
```

### 为什么 cJSON 解析后要释放？

因为 `cJSON_Parse()` 会在堆上分配内存，用完必须：

```c
cJSON_Delete(root);
```

`cJSON_PrintUnformatted()` 返回的字符串也要：

```c
cJSON_free(json_str);
```

---

## 10. 面试项目表达

可以这样介绍：

> 我在 STM32MP157 嵌入式 Linux 开发板上实现了一个 IoT Dashboard 项目。第一阶段将 SSD1306 OLED 的 GPIO 模拟 SPI 控制逻辑模块化，封装出 `oled_init`、`oled_show_string`、`oled_refresh` 等接口，并实现了 6x8 ASCII 字符显示。第二阶段集成 cJSON，将模拟 MQTT payload 解析为带互斥锁保护的全局状态结构体，再刷新到 OLED 屏幕，为后续 MQTT 多线程接入打下基础。

可以重点展开：

- sysfs GPIO 如何控制硬件。
- 软件 SPI 的时序。
- OLED 显存模型。
- cJSON 的解析和内存释放。
- 为什么 `dashboard_state_t` 需要互斥锁。
- Makefile 如何组织多目录交叉编译。

---

## 11. 下一阶段计划

阶段 3：MQTT 通信。

目标：

1. 移植 Paho MQTT C 库。
2. 开发板连接 MQTT Broker。
3. 订阅主题：

```text
stm32mp157/dashboard/control
```

4. 接收 JSON payload。
5. 调用 `json_parse_control()` 更新状态。
6. OLED 实时显示云端下发的数据。

最终链路：

```text
PC/手机发布 MQTT 消息
    ↓
开发板 MQTT 客户端接收
    ↓
cJSON 解析
    ↓
dashboard_state_t 更新
    ↓
OLED 刷新显示
```

---

## 12. 今日复盘 Checklist

完成今天内容后，应能回答：

- [ ] GPIO 编号是怎么从 `GPIOF base=80` 推出来的？
- [ ] 为什么 `SPI_NCS` 最终是 GPIO22？
- [ ] sysfs GPIO 的三个核心文件是什么？
- [ ] 软件 SPI 发送一个字节的流程是什么？
- [ ] `DC` 和 `CS` 分别有什么作用？
- [ ] SSD1306 为什么需要 1024 字节显存？
- [ ] `oled_show_string()` 和 `oled_refresh()` 有什么区别？
- [ ] cJSON 解析后为什么必须 `cJSON_Delete()`？
- [ ] `dashboard_state_t` 为什么需要互斥锁？
- [ ] `snprintf()` 为什么比 `sprintf()` 更安全？
- [ ] Makefile 中 `-I`、`-Wall`、`-MMD` 分别是什么意思？
