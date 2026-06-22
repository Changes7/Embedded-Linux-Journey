# 定义编译器 (根据你的 Ubuntu 环境，通常是这个)
CC = arm-linux-gnueabihf-gcc

# 定义生成的可执行文件名称
TARGET = mp157_app

# 定义你的 NFS 共享目录路径 (根据你之前的截图填写的)
NFS_DIR = ~/nfs_rootfs

# 默认编译目标
all: $(TARGET)

# 编译规则
$(TARGET): main.c
	$(CC) main.c -o $(TARGET) -static
	@echo "编译完成！生成可执行文件: $(TARGET)"

# 部署规则 (将编译好的文件复制到开发板共享目录)
deploy: $(TARGET)
	cp $(TARGET) $(NFS_DIR)/
	@echo "部署完成！已复制到 $(NFS_DIR)"

# 清理规则 (删除编译产物，保持代码库整洁)
clean:
	rm -f $(TARGET)
	@echo "清理完成！"