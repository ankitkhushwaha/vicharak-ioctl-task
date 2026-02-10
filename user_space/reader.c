#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../ioctl_lkd.h"

#define DRIVER_NAME "/dev/ankit"
// #define POP_DATA _IOR('a', 'c', struct data * )

int main(void) {
    int fd = open(DRIVER_NAME, O_RDWR);
    struct data * d = (struct data *)malloc(sizeof(struct data));
    d->length = 3;
    d->data = malloc(3);
    int ret = ioctl(fd, IOCTL_LLKD_IOPOP, d);
    printf("%s\n", d->data);
    close(fd);
    free(d->data);
    free(d);
    return ret;
}
