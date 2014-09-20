#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>

static void usage(const char *cmd)
{
	fprintf(stderr, "Usage: %s <filepath>\n", cmd);
}

static int my_open(const char *filepath);
static void my_close(int d);
static long my_read(int fd);
static void my_write(int fd, long buf);
static long *my_mmap(int fd);
static void my_munmap(long *p);
static void my_msync(long *p);
static void my_socketpair(int sv[2]);
static void my_sendfile(int fd_from, int sd_to);
static long my_recv(int sd);

int main(int argc, char *argv[])
{
	if (2 != argc) {
		usage(argv[0]);
		return 1;
	}
	const char *filepath = argv[1];

	int fd = my_open(filepath);

	char c;
	long *p;
	int sv[2];
	while (1)
		switch (c = getchar()) {
			case '\n':
				break;
			case 'q':
				printf("Bye.\n");
				return 0;
			case 'r':
				p = my_mmap(fd);
				printf("mem-read the first word: %lx\n", *p);
				my_munmap(p);
				break;
			case 'w':
				p = my_mmap(fd);
				*p = 0xbeaf1;
				my_munmap(p);
				printf("mem-written to the first word\n");
				break;
			case 's':
				p = my_mmap(fd);
				*p = 0xbeaf2;
				my_msync(p);
				my_munmap(p);
				printf("mem-written to the first word with msync\n");
				break;
			case 'W':
				my_write(fd, 0xbeaf3);
				printf("write(2)'ed to the first word\n");
				break;
			case 'R':
				printf("read(2)'ed the first word: %lx\n", my_read(fd));
				break;
			case 'S':
				my_socketpair(sv);
				my_sendfile(fd, sv[0]);
				my_close(sv[0]);
				printf("sendfile(2)'ed the first word: %lx\n", my_recv(sv[1]));
				my_close(sv[1]);
				break;
			default:
				printf("Unsupported input '%d'\n", c);
				break;
		}

	return 0;
}

static int my_open(const char *filepath)
{
	int fd = open(filepath, O_RDWR);
	if (-1 == fd) {
		perror("open()");
		exit(EXIT_FAILURE);
	}
	return fd;
}

static void my_close(int d)
{
	if (-1 == close(d)) {
		perror("close()");
		exit(EXIT_FAILURE);
	}
}

static void seekzero(int fd)
{
	if (-1 == lseek(fd, 0, SEEK_SET)) {
		perror("lseek()");
		exit(EXIT_FAILURE);
	}
}

static long my_read(int fd)
{
	long buf;
	if (-1 == read(fd, &buf, sizeof(buf))) {
		perror("read()");
		exit(EXIT_FAILURE);
	}
	seekzero(fd);
	return buf;
}

static void my_write(int fd, long buf)
{
	if (-1 == write(fd, &buf, sizeof(buf))) {
		perror("write()");
		exit(EXIT_FAILURE);
	}
	seekzero(fd);
}

static long *my_mmap(int fd)
{
	void *p = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (MAP_FAILED == p) {
		perror("mmap()");
		exit(EXIT_FAILURE);
	}
	return (long *)p;
}

static void my_munmap(long *p)
{
	if (-1 == munmap(p, 4096)) {
		perror("munmap()");
		exit(EXIT_FAILURE);
	}
}

static void my_msync(long *p)
{
	if (-1 == msync(p, 4096, MS_SYNC)) {
		perror("msync()");
		exit(EXIT_FAILURE);
	}
}

static void my_socketpair(int sv[2])
{
	if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) {
		perror("socketpair()");
		exit(EXIT_FAILURE);
	}
}

static void my_sendfile(int fd_from, int sd_to)
{
	off_t off = 0;
	if (-1 == sendfile(sd_to, fd_from, &off, 100)) {
		perror("sendfile()");
		exit(EXIT_FAILURE);
	}
	assert(100 == off);
	seekzero(fd_from);
}

static long my_recv(int sd)
{
	long buf;
	if (-1 == recv(sd, &buf, sizeof(buf), 0)) {
		perror("recv()");
		exit(EXIT_FAILURE);
	}
	return buf;
}
