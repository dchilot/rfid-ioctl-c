#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * $CC -O2 -w main.c -o main
 */

int IIC_IO = 0xc0036909;
int len_getData = 6;
signed char buf_getData[] = {0, 0, 5, 1, 2, 0x42, 0, 0, 0, 0};
int len_getStatus = 7;
signed char buf_getStatus[] = {0, 0, 5, 1, 2, 0x32, 0};
int const SLEEP = 350000;
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
	if (result <= 0)
	{
		exit_extra(result, "write");
	}
}

void ioctl_or_die(int fd, unsigned long int request, char * buf, int len)
{
	int result;
	char * buf_copy;
	int i;

	buf_copy = (char *)malloc(sizeof(char) * len);
	memcpy(buf_copy, buf, len);
	result = ioctl(fd, request, buf_copy);
	free(buf_copy);
	if (0 > result)
	{
		exit_extra(result, "ioctl");
	}
}

void ioctl_getStatus(int fd)
{
	int result;
	char * buf_copy;
	int i;

	buf_copy = (char *)malloc(sizeof(char) * len_getStatus);
	memcpy(buf_copy, buf_getStatus, len_getStatus);
	result = ioctl(fd, IIC_IO, buf_copy);
	printf("result = %i\n", result);
	printf("status = %i\n", buf_copy[len_getStatus - 1]);
	free(buf_copy);
	if (0 > result)
	{
		exit_extra(result, "ioctl (getStatus)");
	}
}

void read_value(int fd)
{
	int result;
	int i;
	char buf_copy[10];

	usleep(SLEEP);
	printf("ioctl get value\n");
	memcpy(buf_copy, buf_getData, len_getData);
	result = ioctl(fd, IIC_IO, buf_copy);
	if (0 > result)
	{
		exit_extra(result, "ioctl (getData)");
	}
	else
	{
		printf("value =");
		for (i = 0 ; i < 5 ; ++i)
		{
			printf(" %hhX", buf_copy[5 + i]);
		}
		printf("\n");
	}
}

int main()
{
	int len_connected = 2;
	signed char buf_connected[] = {0, 'c'};
	int len_float = 2;
	signed char buf_float[] = {0, 'f'};
	int len_stop = 2;
	signed char buf_stop[] = {0, 'd'};
	int len_init = 2;
	signed char buf_init[] = {0, 51};
	signed char buf_single[] = {0x41, 1};
	signed char buf_continuous[] = {0x41, 2};
	int len_X = 6;
	signed char buf_0_0_0_2_2_0[] = {0, 0, 0, 2, 2, 0};
	signed char buf_0_0_0_2_2_41[] = {0, 0, 0, 2, 2, 0x41};
	signed char buf_0_0_5_1_2_42[] = {0, 0, 5, 1, 2, 0x42};
	int len_port = 1;
	signed char buf_port[] = {0};
	int IIC_CONNECT = 0xc0036907;
	int fd_iic = 0;
	int fd_dcm = 0;
	int i = 0;
	int j = 0;
	int result = 0;
	printf("open\n");
	fd_iic = open_or_die("/dev/lms_iic", 2);
	FDS[0] = fd_iic;
	printf("fd_iic = %i\n", fd_iic);
	if (0)
	{
		fd_dcm = fd_iic;
	}
	else
	{
		fd_dcm = open_or_die("/dev/lms_dcm", 2);
		FDS[1] = fd_dcm;
		printf("fd_dcm = %i\n", fd_dcm);
	}
	printf("write connected\n");
	write_or_die(fd_dcm, buf_connected, len_connected);
	for (i = 0 ; i < 2 ; ++i)
	{
		printf("write float\n");
		write_or_die(fd_dcm, buf_float, len_float);
	}
	printf("write init\n");
	write_or_die(fd_dcm, buf_init, len_init);
	usleep(SLEEP);
	printf("ioctl connect\n");
	ioctl_or_die(fd_iic, IIC_CONNECT, buf_port, len_port);
	usleep(SLEEP);
	printf("ioctl buf_0_0_0_2_2_0 * 3\n");
	for (i = 0 ; i < 3 ; ++i)
	{
		usleep(2 * SLEEP);
		ioctl_or_die(fd_iic, IIC_IO, buf_0_0_0_2_2_0, len_X);
	}
	printf("ioctl buf_0_0_0_2_2_41\n");
	ioctl_or_die(fd_iic, IIC_IO, buf_0_0_0_2_2_41, len_X);
	printf("ioctl buf_0_0_5_1_2_42 * 2\n");
	for (i = 0 ; i < 2 ; ++i)
	{
		usleep(2 * SLEEP);
		ioctl_or_die(fd_iic, IIC_IO, buf_0_0_5_1_2_42, len_X);
	}
	/*printf("ioctl set single read\n");*/
	/*ioctl_or_die(fd_iic, IIC_IO, buf_single);*/
	/*result = read_value(fd_iic);*/
	/*if (0 > result)*/
	/*{*/
	/*return 1;*/
	/*}*/
	usleep(10 * SLEEP);
	printf("ioctl get status\n");
	ioctl_getStatus(fd_iic);
	printf("ioctl buf_0_0_0_2_2_41\n");
	ioctl_or_die(fd_iic, IIC_IO, buf_0_0_0_2_2_41, len_X);
	printf("ioctl buf_0_0_5_1_2_42\n");
	ioctl_or_die(fd_iic, IIC_IO, buf_0_0_5_1_2_42, len_X);

	/*printf("ioctl set continuous read\n");*/
	/*result = ioctl(fd_iic, IIC_IO, buf_continuous);*/
	if (0 > result)
	{
		exit_extra(result, "ioctl");
	}
	usleep(SLEEP);
	printf("ioctl get status\n");
	ioctl_getStatus(fd_iic);
	for (j = 0 ; j < 10 ; ++j)
	{
		read_value(fd_iic);
	}
	printf("write stop\n");
	write_or_die(fd_dcm, buf_stop, len_stop);
	close(fd_dcm);
	close(fd_iic);
	return 0;
}
