#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../ioctl_lkd.h"

#define DRIVER_NAME "/dev/ankit"
// #define SET_SIZE_OF_QUEUE _IOW('a', 'a', int * )

int main(void) {
    int fd = open(DRIVER_NAME, O_RDWR);
    printf("fd: %d", fd);
    int size = 100;
    int ret = ioctl(fd, IOCTL_LLKD_IOSIZE, size);
    printf("ret: %d", ret);
    close(fd);
    return ret;
}
