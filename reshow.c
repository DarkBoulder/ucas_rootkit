#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char ** argv)
{
    if (argc < 2)
        return 0;
    int fd = open("/dev/intel_rapl_msrdv", O_RDWR);
    ioctl(fd, 0x1002, atoi(argv[1]));
}
