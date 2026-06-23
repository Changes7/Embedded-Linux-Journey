CC = /opt/toolchain/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc
NFS_DIR = ~/nfs_rootfs

# 定义多个编译目标，让你的 Mono-repo 更加专业！
TARGETS = mp157_app key_app

all: $(TARGETS)

# 目标一：LED 命令行控制程序
mp157_app: main.c
	$(CC) main.c -o mp157_app
	@echo "编译完成：mp157_app"

# 目标二：按键事件监听程序
key_app: key_test.c
	$(CC) key_test.c -o key_app
	@echo "编译完成：key_app"

# 部署规则：一键将所有可执行文件打包复制到共享目录
deploy: $(TARGETS)
	cp $(TARGETS) $(NFS_DIR)/
	@echo "部署完成！已将所有可执行文件复制到 $(NFS_DIR)/"

clean:
	rm -f $(TARGETS)
	@echo "清理完成！"
