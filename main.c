#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int IIC_IO = 0xc0036909;
signed char buf_getData[] = {0, 0, 5, 1, 2, 0x42, 0, 0, 0, 0};
int const SLEEP = 250000;
int FDS [] = {-1, -1};
int const FDS_LEN = 2;


void exit_extra(
		int result,
		char * what)
{
	int i;

	printf("result = %c\n", result);
	printf("errno = %d\n", errno);
	printf("Oh dear, something went wrong! %s\n", strerror(errno));
	printf("failed to %s - abort\n", what);
	for (i = 0 ; i < FDS_LEN ; ++i)
	{
		if (FDS[i] > 0)
		{
			close(FDS[i]);
		}
	}
	exit(1);
}

int open_or_die(char * filename, int mode)
{
	int fd;

	fd = open(filename, mode);
	if (fd <= 0)
	{
		exit_extra(fd, "open");
	}
	return fd;
}

void write_or_die(int fd, char * buf, int len)
{
    int result;
    result = write(fd, buf, len);
	exit_extra(result, "write");
}

int read_value(int fd)
{
    int result;
    int i;
    usleep(SLEEP);
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
    int fd_iic = 0;
    int fd_dcm = 0;
    int i = 0;
    int j = 0;
    int result = 0;
    printf("open\n");
    fd_iic = open_or_die("/dev/lms_iic", 2);
    printf("fd_iic = %i\n", fd_iic);
    fd_dcm = open_or_die("/dev/lms_dcm", 2);
    printf("fd_dcm = %i\n", fd_dcm);
    printf("write connected\n");
    write_or_die(fd_dcm, buf_connected, len_connected);
    for (i = 0 ; i < 2 ; ++i)
    {
        printf("write float\n");
        write_or_die(fd_dcm, buf_float, len_float);
    }
    printf("ioctl connect\n");
    result = ioctl(fd_iic, IIC_CONNECT, buf_port);
    if (0 > result)
    {
		exit_extra(result, "ioctl");
    }
    printf("write init\n");
    write_or_die(fd_dcm, buf_init, len_init);
    usleep(SLEEP);
    printf("ioctl set single read\n");
    result = ioctl(fd_iic, IIC_IO, buf_single);
    if (0 > result)
    {
		exit_extra(result, "ioctl");
    }
    result = read_value(fd_iic);
    if (0 > result)
    {
        return 1;
    }
    usleep(SLEEP);
    printf("ioctl set continuous read\n");
    result = ioctl(fd_iic, IIC_IO, buf_continuous);
    if (0 > result)
    {
		exit_extra(result, "ioctl");
    }
    usleep(SLEEP);
    printf("ioctl get status\n");
    result = ioctl(fd_iic, IIC_IO, buf_getStatus);
    if (0 > result)
    {
		exit_extra(result, "ioctl");
    }
    printf("status = %d\n", buf_getStatus[5]);
    for (j = 0 ; j < 10 ; ++j)
    {
        result = read_value(fd_iic);
        if (0 > result)
        {
            return 1;
        }
    }
    close(fd_iic);
    return 0;
}
