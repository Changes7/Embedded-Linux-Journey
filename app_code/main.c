#include <stdio.h>
#include <unistd.h>  // 包含 getpid() 等 POSIX 操作系统 API
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // 打印带有特定硬件标识的问候语
    printf("======================================\n");
    printf(" Hello STM32MP157! \n");
    printf(" 欢迎开启 Linux 嵌入式开发之旅 \n");
    printf("======================================\n");

    // 获取并打印当前进程的 ID (PID)
    pid_t current_pid = getpid();
    printf("[INFO] 当前程序的运行进程 PID: %d\n", current_pid);
    
    // 检查是否有通过命令行传入参数
    if (argc > 1) {
        printf("[INFO] 接收到额外的命令行参数: %s\n", argv[1]);
    }

    return EXIT_SUCCESS;
}