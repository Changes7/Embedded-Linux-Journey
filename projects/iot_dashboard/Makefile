# IoT Dashboard - 顶级工程构建文件
# 支持多目录联合交叉编译

# ─── 工具链配置 ───────────────────────────────────────────────────────
CC      = /opt/toolchain/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc
CFLAGS  = -Wall -Wextra -O2 -I./include -I./third_party/cJSON
LDFLAGS = -lpthread

# ─── 目录定义 ─────────────────────────────────────────────────────────
SRC_DIR     = src
THIRD_DIR   = third_party/cJSON
BUILD_DIR   = build
DEPLOY_DIR  = $(HOME)/nfs_rootfs

# ─── 源文件收集 ───────────────────────────────────────────────────────
SRCS  = $(wildcard $(SRC_DIR)/*.c)
SRCS += $(wildcard $(THIRD_DIR)/*.c)
OBJS  = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRCS)))

# ─── 目标 ─────────────────────────────────────────────────────────────
TARGET = $(BUILD_DIR)/iot_dashboard

.PHONY: all clean deploy dirs

all: dirs $(TARGET)
	@echo "[OK] 编译完成: $(TARGET)"

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 编译 src/ 下的 .c
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 编译 third_party/cJSON/ 下的 .c
$(BUILD_DIR)/%.o: $(THIRD_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

dirs:
	@mkdir -p $(BUILD_DIR)

deploy: all
	@mkdir -p $(DEPLOY_DIR)
	cp $(TARGET) $(DEPLOY_DIR)/
	@echo "[DEPLOY] 已部署到 $(DEPLOY_DIR)/iot_dashboard"

clean:
	rm -rf $(BUILD_DIR)
	@echo "[CLEAN] 已清理构建产物"
