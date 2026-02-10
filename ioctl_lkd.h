#include <linux/types.h>

struct data {
    int length;
    char * data;
};

#define IOCTL_LLKD_MAGIC        'a'

#define IOCTL_LLKD_MAXIOCTL     3

#define IOCTL_LLKD_IOSIZE     _IOR(IOCTL_LLKD_MAGIC, 0, int)

#define IOCTL_LLKD_IOPUSH   _IOW(IOCTL_LLKD_MAGIC, 1, struct data *)

#define IOCTL_LLKD_IOPOP    _IOR(IOCTL_LLKD_MAGIC, 2, struct data *)