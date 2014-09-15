#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static void usage(const char *cmd)
{
	fprintf(stderr, "Usage: %s <filepath>\n", cmd);
}

int main(int argc, char *argv[])
{
	if (2 != argc) {
		usage(argv[0]);
		return 1;
	}
	const char *filepath = argv[1];

	int fd = open(filepath, O_RDWR);
	if (-1 == fd) {
		perror("open()");
		return 1;
	}

	void *p = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (MAP_FAILED == p) {
		perror("mmap()");
		return 1;
	}

	char c;
	while (1)
		switch (c = getchar()) {
			case 'q':
				printf("Bye.\n");
				return 0;
			case 'r':
				printf("read first 4bytes: %d\n", *(int *)p);
				break;
			case 'w':
				*(int *)p = (int)c;
				printf("written to first 4bytes\n");
				break;
			case 's':
				if (-1 == msync(p, 4096, MS_SYNC)) {
					perror("msync()");
					return 1;
				}
				printf("synchronized.\n");
				break;
			case 'W':
				if (-1 == write(fd, &c, sizeof(c))) {
					perror("write()");
					return 1;
				}
				printf("write(2)'ed to first 1byte\n");
				break;
			case 'R':
				if (-1 == read(fd, &c, sizeof(c))) {
					perror("read()");
					return 1;
				}
				printf("read(2)'ed first 1byte\n");
				break;
			default:
				printf("Unsupported input '%d'\n", c);
				break;
		}

	return 0;
}
