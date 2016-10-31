#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define __USE_GNU
#include <dlfcn.h>

/*
 * $CC -O2 -w preload.c -o preload -ldl -L $INSTALLDIR/lib/ -I $INSTALLDIR/include -fPIC -shared
 */

typedef int (*orig_open_f_type)(const char *pathname, int flags, ...);
typedef int (*orig_close_f_type)(int fd);
typedef ssize_t (*orig_write_f_type)(int fd, const void *buf, size_t count);
typedef ssize_t (*orig_pwrite_f_type)(int fd, const void *buf, size_t count, off_t offset);
typedef int (*orig_ioctl_f_type)(int fd, unsigned long int request, ...);



int G_FD = -1;
signed char G_PATHNAME[] = "/dev/lms_iic";
ssize_t G_LEN = 0;


int open(const char *pathname, int flags, ...)
{
	va_list ap;
	int fd;
	int min_len;
	int len;
	orig_open_f_type orig_open;

	orig_open = (orig_open_f_type)dlsym(RTLD_NEXT, "open");
	va_start(ap, flags);
	fd = orig_open(pathname, flags, ap);
	va_end(ap);
	if (0 == G_LEN)
	{
		G_LEN = strlen(G_PATHNAME);
	}
	len = strlen(pathname);
	printf("CALL open(\"%s\", %i, ...) -> %i\n", pathname, flags, fd);
	if (len == G_LEN)
	{
		if (0 == strncmp(pathname, G_PATHNAME, G_LEN))
		{
			G_FD = fd;
		}
	}
	return fd;
}

int close(int fd)
{
	orig_close_f_type orig_close;

	/*if ((G_FD > 0) && (G_FD == fd))*/
	{
		printf("CALL close(%i)\n", fd);
	}

	orig_close = (orig_close_f_type)dlsym(RTLD_NEXT, "close");
	return orig_close(fd);
}

ssize_t write(int fd, const void *buf, size_t count)
{
	orig_write_f_type orig_write;
	char * c;
	size_t partial_count;
	int i;

	/*if ((G_FD > 0) && (G_FD == fd))*/
	{
		printf("CALL write(%i,", fd);
		c = buf;
		if (count < 10)
		{
			partial_count = count;
		}
		else
		{
			partial_count = 10;
		}
		for (i = 0 ; i < partial_count ; ++i)
		{
			printf(" %hhX", c[i]);
		}
		if (count > 10)
		{
			printf(" (...)");
		}
		printf(", %zu)\n", count);
	}

	orig_write = (orig_write_f_type)dlsym(RTLD_NEXT, "write");
	return orig_write(fd, buf, count);
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	orig_pwrite_f_type orig_pwrite;
	size_t partial_count;
	char * c;
	int i;

	/*if ((G_FD > 0) && (G_FD == fd))*/
	{
		printf("CALL pwrite(%i,", fd);
		c = buf;
		if (count < 10)
		{
			partial_count = count;
		}
		else
		{
			partial_count = 10;
		}
		for (i = 0 ; i < partial_count ; ++i)
		{
			printf(" %hhX", c[i]);
		}
		if (count > 10)
		{
			printf(" (...)");
		}
		printf("%zu, %jd)\n", count, offset);
	}

	orig_pwrite = (orig_pwrite_f_type)dlsym(RTLD_NEXT, "pwrite");
	return orig_pwrite(fd, buf, count, offset);
}

int ioctl(int fd, unsigned long int request, ...)
{
	va_list ap;
	void *argp;
	char * c;
	int status;
	orig_ioctl_f_type orig_ioctl;

	orig_ioctl = (orig_ioctl_f_type)dlsym(RTLD_NEXT, "ioctl");
	/*va_start(ap, request);*/
	/*return status;*/
	/*va_end(ap);*/
	va_start(ap, request);
	argp = va_arg(ap, void *);
	va_end(ap);
	/*if ((G_FD > 0) && (G_FD == fd))*/
	{
		c = argp;
		printf("CALL ioctl(%i, %lx, %p [%hhX %hhX %hhX %hhX %hhX %hhX]) -> ",
			 fd, request, argp, c[0], c[1], c[2], c[3], c[4], c[5]);
	}
	status = orig_ioctl(fd, request, argp);
	/*if ((G_FD > 0) && (G_FD == fd))*/
	{
		printf("%i | [%hhX %hhX %hhX %hhX %hhX]\n", status, c[5], c[6], c[7], c[8], c[9]);
	}
	return status;
}
