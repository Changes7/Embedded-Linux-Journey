#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define DEVICE_PATH "/dev/my_device"
#define BUFFER_SIZE 256

static void print_usage(const char *prog)
{
    fprintf(stderr, "Usage: %s [write_string]\n", prog);
    fprintf(stderr, "Example: %s \"Hello STM32MP157\"\n", prog);
}

int main(int argc, char *argv[])
{
    int fd;
    ssize_t ret;
    char read_buf[BUFFER_SIZE];
    const char *write_msg;

    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    write_msg = argv[1];

    printf("======================================\n");
    printf("  Character Device Test (/dev/my_device)\n");
    printf("======================================\n");

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "[ERROR] open %s failed: %s\n",
                DEVICE_PATH, strerror(errno));
        fprintf(stderr, "[HINT] 请先加载驱动: sudo insmod my_device_driver.ko\n");
        return EXIT_FAILURE;
    }
    printf("[INFO] open %s success, fd=%d\n", DEVICE_PATH, fd);

    ret = write(fd, write_msg, strlen(write_msg));
    if (ret < 0) {
        perror("[ERROR] write failed");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("[INFO] write %zd bytes: \"%s\"\n", ret, write_msg);

    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("[ERROR] lseek failed");
        close(fd);
        return EXIT_FAILURE;
    }

    memset(read_buf, 0, sizeof(read_buf));
    ret = read(fd, read_buf, sizeof(read_buf) - 1);
    if (ret < 0) {
        perror("[ERROR] read failed");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("[INFO] read %zd bytes: \"%s\"\n", ret, read_buf);

    if (strcmp(write_msg, read_buf) == 0) {
        printf("[PASS] write/read data match!\n");
    } else {
        printf("[FAIL] data mismatch!\n");
        printf("       expected: \"%s\"\n", write_msg);
        printf("       got:      \"%s\"\n", read_buf);
    }

    close(fd);
    printf("[INFO] device closed, test done.\n");
    return EXIT_SUCCESS;
}
