#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int IIC_IO = 0xc0036909;
signed char buf_getData[] = {0, 0, 5, 1, 2, 0x42, 0, 0, 0, 0};

int write_buf(int fd, char * buf, int len)
{
    int result;
    result = write(fd, buf, len);
    if (-1 == result)
    {
        printf("result = %c\n", result);
        printf("errno = %d\n", errno);
        printf("Oh dear, something went wrong! %s\n", strerror(errno));
        printf("failed to write - abort\n");
        close(fd);
    }
    return result;
}

int read_value(int fd)
{
    int result;
    int i;
    usleep(250000);
    printf("ioctl get value\n");
    result = ioctl(fd, IIC_IO, buf_getData);
    if (0 > result)
    {
        printf("result = %d\n", result);
        printf("errno = %d\n", errno);
        printf("Oh dear, something went wrong! %s\n", strerror(errno));
        printf("failed to ioctl - abort\n");
        close(fd);
        result = -1;
    }
    else
    {
        printf("value =");
        for (i = 0 ; i < 5 ; ++i)
        {
            printf(" %d", buf_getData[5 + i]);
        }
        printf("\n");
    }
    return result;
}

int main()
{
    int len_connected = 2;
    signed char buf_connected[] = {0, 'c'};
    int len_float = 2;
    signed char buf_float[] = {0, 'f'};
    int len_init = 2;
    signed char buf_init[] = {0, 51};
    signed char buf_single[] = {0x41, 1};
    signed char buf_continuous[] = {0x41, 2};
    signed char buf_getStatus[] = {0, 0, 5, 1, 2, 0x32, 0};
    signed char buf_port[] = {0};
    int IIC_CONNECT = 0xc0036907;
    int fd = 0;
    int i = 0;
    int j = 0;
    int result = 0;
    printf("open\n");
    fd = open("/dev/lms_iic", 2);
    if (0 == fd)
    {
        printf("failed to open - abort\n");
        return 1;
    }
    printf("fd = %i\n", fd);
    printf("write connected\n");
    result = write_buf(fd, buf_connected, len_connected);
    if (-1 == result)
    {
        return 1;
    }
    for (i = 0 ; i < 2 ; ++i)
    {
        printf("write float\n");
        result = write_buf(fd, buf_float, len_float);
        if (-1 == result)
        {
            return 1;
        }
    }
    printf("ioctl connect\n");
    result = ioctl(fd, IIC_CONNECT, buf_port);
    if (0 > result)
    {
        printf("result = %d\n", result);
        printf("errno = %d\n", errno);
        printf("Oh dear, something went wrong! %s\n", strerror(errno));
        printf("failed to ioctl - abort\n");
        close(fd);
        return 1;
    }
    printf("write\n");
    result = write_buf(fd, buf_init, len_init);
    if (-1 == result)
    {
        return 1;
    }
    usleep(250000);
    printf("ioctl set single read\n");
    result = ioctl(fd, IIC_IO, buf_single);
    if (0 > result)
    {
        printf("result = %d\n", result);
        printf("errno = %d\n", errno);
        printf("Oh dear, something went wrong! %s\n", strerror(errno));
        printf("failed to ioctl - abort\n");
        close(fd);
        return 1;
    }
    result = read_value(fd);
    if (0 > result)
    {
        return 1;
    }
    usleep(250000);
    printf("ioctl set continuous read\n");
    result = ioctl(fd, IIC_IO, buf_continuous);
    if (0 > result)
    {
        printf("result = %d\n", result);
        printf("errno = %d\n", errno);
        printf("Oh dear, something went wrong! %s\n", strerror(errno));
        printf("failed to ioctl - abort\n");
        close(fd);
        return 1;
    }
    usleep(250000);
    printf("ioctl get status\n");
    result = ioctl(fd, IIC_IO, buf_getStatus);
    if (0 > result)
    {
        printf("result = %d\n", result);
        printf("errno = %d\n", errno);
        printf("Oh dear, something went wrong! %s\n", strerror(errno));
        printf("failed to ioctl - abort\n");
        close(fd);
        return 1;
    }
    printf("status = %d\n", buf_getStatus[5]);
    for (j = 0 ; j < 10 ; ++j)
    {
        result = read_value(fd);
        if (0 > result)
        {
            return 1;
        }
    }
    close(fd);
    return 0;
}
