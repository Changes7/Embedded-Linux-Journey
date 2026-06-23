#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

int main(int argc, char *argv[]) {
    int fd;
    struct input_event ev;

    // 1. 利用命令行参数传入设备节点，复用 argv 核心知识！
    if (argc < 2) {
        printf("======================================\n");
        printf("[错误] 缺少输入设备节点参数！\n");
        printf("用法 (Usage): sudo %s <设备节点>\n", argv[0]);
        printf("示例 (Example): sudo %s /dev/input/event1\n", argv[0]);
        printf("======================================\n");
        return EXIT_FAILURE;
    }

    // 2. 以只读方式 (O_RDONLY) 打开按键对应的输入子系统事件节点
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("[ERROR] 无法打开输入设备，请检查节点是否存在或使用 sudo 运行");
        return EXIT_FAILURE;
    }

    printf("成功监听设备 %s，请按下开发板上的物理按键 (按 Ctrl+C 退出)...\n", argv[1]);

    // 3. 循环读取事件数据流
    while (1) {
        /*
         * 💡 OS 核心设计点：阻塞式读取 (Blocking Read)
         * 当按键没有动作时，由于无数据可读，当前进程会被内核拉入等待队列并休眠，CPU 占用率归 0。
         * 只有当按键按下或释放产生硬件中断时，内核才会唤醒此进程，并填充 ev 结构体。
         */
        if (read(fd, &ev, sizeof(struct input_event)) < (ssize_t)sizeof(struct input_event)) {
            perror("[ERROR] 读取事件失败");
            close(fd);
            return EXIT_FAILURE;
        }

        // 4. 解析 input 子系统事件
        // type == EV_KEY (值为 1) 表示该事件是按键类事件
        if (ev.type == EV_KEY) {
            printf("[按键动作] 键码(Code): %3d | 状态(Value): %d (%s)\n", 
                   ev.code, 
                   ev.value, 
                   ev.value == 1 ? "按下 ⬇️" : (ev.value == 0 ? "释放 ⬆️" : "连按 🔄"));
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}