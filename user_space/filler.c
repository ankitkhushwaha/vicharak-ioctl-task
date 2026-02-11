#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#include "../ioctl_lkd.h"

#define DRIVER_NAME "/dev/ankit"
// #define PUSH_DATA _IOW('a', 'b', struct data * )

int main(void)
{
    int fd = open(DRIVER_NAME, O_RDWR);
    struct data *d = (struct data *)malloc(sizeof(struct data));
    d->length = 3;
    memcpy(d->data, "xyz", 3);
    int ret = ioctl(fd, IOCTL_LLKD_IOPUSH, d);
    printf("ret= %d\n", ret);
    close(fd);
    free(d);
    return ret;
}
