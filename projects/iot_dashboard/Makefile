# IoT Dashboard - 顶级工程构建文件
# 支持多目录联合交叉编译

# ─── 工具链配置 ───────────────────────────────────────────────────────
CC      = /opt/toolchain/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc
CFLAGS  = -Wall -Wextra -O2 -I./include -I./third_party/cJSON -MMD -MP
LDFLAGS = -lpthread
# LDFLAGS += -lpaho-mqtt3c  # 阶段 3 移植 MQTT 后再启用

# ─── 目录定义 ─────────────────────────────────────────────────────────
SRC_DIR     = src
THIRD_DIR   = third_party/cJSON
BUILD_DIR   = build
DEPLOY_DIR  = $(HOME)/nfs_rootfs

# ─── 源文件收集 ───────────────────────────────────────────────────────
# 1. 排除 cJSON 里面不需要编译的 test 或 example 文件
THIRD_SRCS = $(filter-out $(THIRD_DIR)/test.c $(THIRD_DIR)/examples/%, $(wildcard $(THIRD_DIR)/*.c))
SRCS  = $(wildcard $(SRC_DIR)/*.c)
SRCS += $(THIRD_SRCS)

OBJS  = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRCS)))
DEPS  = $(OBJS:.o=.d)

# ─── 目标 ─────────────────────────────────────────────────────────────
TARGET = $(BUILD_DIR)/iot_dashboard

.PHONY: all clean deploy dirs

all: dirs $(TARGET)
	@echo "[OK] 编译完成: $(TARGET)"

# 3. 链接阶段
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 4. 编译 src/ 下的 .c
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 5. 编译 third_party/cJSON/ 下的 .c
$(BUILD_DIR)/%.o: $(THIRD_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 6. 包含自动生成的依赖文件（解决修改.h导致不重新编译的问题）
-include $(DEPS)

dirs:
	@mkdir -p $(BUILD_DIR)

deploy: all
	@mkdir -p $(DEPLOY_DIR)
	cp $(TARGET) $(DEPLOY_DIR)/
	@echo "[DEPLOY] 已部署到 $(DEPLOY_DIR)/iot_dashboard"

clean:
	rm -rf $(BUILD_DIR)
	@echo "[CLEAN] 已清理构建产物"