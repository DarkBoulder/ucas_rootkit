#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char ** argv)
{
    int fd = open("/dev/intel_rapl_msrdv", O_RDWR);
    sleep(10);
    ioctl(fd, 0x1001);
    sleep(114514);
}

