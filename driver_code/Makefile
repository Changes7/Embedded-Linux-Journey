# 1. 声明要编译的目标模块。kbuild 引擎会自动寻找对应的 hello_driver.c 编译为 .ko
obj-m += hello_driver.o

# 2. 定义开发板对应的 Linux 内核源码树目录（非常关键！）
# ⚠️ 编译内核模块必须依赖已经配置、编译过的内核源码。
# 请根据你的 Ubuntu 虚拟机中内核源码树的具体位置修改此路径（通常在 ~/linux/ebf_linux_kernel 之类的路径下）
KDIR ?= /home/work/system/linux

# 3. 获取当前目录绝对路径
PWD := $(shell pwd)

# 4. 定义交叉编译器前缀与 CPU 架构
ARCH = arm
CROSS_COMPILE = arm-none-linux-gnueabihf-

all:
	@echo "开始编译内核模块..."
	$(MAKE) -C $(KDIR) M=$(PWD) modules ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
	@echo "内核模块编译完成！"

# 一键部署：把编译好的 .ko 驱动直接推送到共享目录
deploy: all
	cp hello_driver.ko ~/nfs_rootfs/
	@echo "部署完成！.ko 文件已成功复制到 ~/nfs_rootfs/ 目录"

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@echo "清理临时编译产物完成！"