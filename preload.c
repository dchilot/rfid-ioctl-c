#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fs.h>
#include <linux/types.h>

#define __USE_GNU
#include <dlfcn.h>

/*
 * $CC -O2 -w preload.c -o preload -ldl -fPIC -shared
 */

typedef int (*orig_open_f_type)(const char *pathname, int flags, ...);
typedef int (*orig_close_f_type)(int fd);
typedef ssize_t (*orig_write_f_type)(int fd, const void *buf, size_t count);
typedef ssize_t (*orig_pwrite_f_type)(int fd, const void *buf, size_t count, off_t offset);
typedef int (*orig_ioctl_f_type)(int fd, unsigned long int request, ...);
typedef void * (*orig_mmap_f_type)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
typedef int (*orig_handleOpen_f_type)(const char *path, int oflag, int mode);
typedef int (*orig_filp_open_f_type)(const char *filename, int flags, umode_t mode);
typedef FILE * (*orig_fopen_f_type)(const char *path, const char *mode);


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
	if (1 != fd)
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

void * mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	orig_mmap_f_type orig_mmap;
    void * result;

	orig_mmap = (orig_mmap_f_type)dlsym(RTLD_NEXT, "mmap");
	result = orig_mmap(addr, length, prot, flags, fd, offset);
    printf("CALL mmap(%p, %zu, %i, %i, %i, %jd) -> %p\n", addr, length, prot, flags, fd, offset, result);
    return result;
}

int handleOpen(const char *path, int oflag, int mode)
{
    int fd;
	orig_handleOpen_f_type orig_handleOpen;

	orig_handleOpen = (orig_handleOpen_f_type)dlsym(RTLD_NEXT, "handleOpen");
	fd = orig_handleOpen(path, oflag, mode);
	printf("CALL handleOpen(\"%s\", %i, %i) -> %i\n", path, oflag, mode, fd);
	return fd;
}

int sys_open(const char *path, int oflag, int mode)
{
    int fd;
	orig_handleOpen_f_type orig_sys_open;

	orig_sys_open = (orig_handleOpen_f_type)dlsym(RTLD_NEXT, "sys_open");
	fd = orig_sys_open(path, oflag, mode);
	printf("CALL sys_open(\"%s\", %i, %i) -> %i\n", path, oflag, mode, fd);
	return fd;
}

struct file *filp_open(const char *filename, int flags, umode_t mode)
{
    struct file * result;
	orig_filp_open_f_type orig_filp_open;

	orig_filp_open = (orig_filp_open_f_type)dlsym(RTLD_NEXT, "filp_open");
	result = orig_filp_open(filename, flags, mode);
	printf("CALL filp_open(\"%s\", %i, %hu) -> %i\n", filename, flags, mode, fileno(result));
	return result;
}

FILE *fopen(const char *path, const char *mode)
{
	FILE * result;
	orig_fopen_f_type orig_fopen;

	orig_fopen = (orig_fopen_f_type)dlsym(RTLD_NEXT, "fopen");
	printf("CALL fopen(\"%s\", \"%s\")\n", path, mode);
	return orig_fopen(path, mode);
	/*result = orig_fopen(path, mode);*/
	/*printf("CALL fopen(\"%s\", \"%s\") -> %i\n", path, mode, fileno(result));*/
	/*return result;*/
}
