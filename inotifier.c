#include <stdio.h>
#include <assert.h>
#include <sys/inotify.h>
#include <unistd.h>

static void usage(const char *cmd)
{
	fprintf(stderr, "%s <filepath>\n", cmd);
}

static inline unsigned long __ffs(unsigned long word)
{
	asm("rep; bsf %1,%0"
			: "=r" (word)
			: "rm" (word));
	return word;
}

#define for_each_bit(bit, bits) \
	for (bit = __ffs(bits); (bits); bit = __ffs((bits) &= ~(1 << bit)))

const struct { unsigned long flag; char desc[100]; } flag_descs[] = {
#define ENTRY(name, desc) { name, #name " : " desc }
/* Supported events suitable for MASK parameter of INOTIFY_ADD_WATCH.  */
ENTRY(IN_ACCESS, "File was accessed. "),
ENTRY(IN_MODIFY, "File was modified. "),
ENTRY(IN_ATTRIB, "Metadata changed. "),
ENTRY(IN_CLOSE_WRITE, "Writtable file was closed. "),
ENTRY(IN_CLOSE_NOWRITE, "Unwrittable file closed. "),
ENTRY(IN_CLOSE, "Close. "),
ENTRY(IN_OPEN, "File was opened. "),
ENTRY(IN_MOVED_FROM, "File was moved from X. "),
ENTRY(IN_MOVED_TO, "File was moved to Y. "),
ENTRY(IN_MOVE, "Moves. "),
ENTRY(IN_CREATE, "Subfile was created. "),
ENTRY(IN_DELETE, "Subfile was deleted. "),
ENTRY(IN_DELETE_SELF, "Self was deleted. "),
ENTRY(IN_MOVE_SELF, "Self was moved. "),

/* Events sent by the kernel.  */
ENTRY(IN_UNMOUNT, "Backing fs was unmounted. "),
ENTRY(IN_Q_OVERFLOW, "Event queued overflowed. "),
ENTRY(IN_IGNORED, "File was ignored. "),

/* Helper events.  */
ENTRY(IN_CLOSE, "Close. "),
ENTRY(IN_MOVE, "Moves. "),

/* Special flags.  */
ENTRY(IN_ONLYDIR, "Only watch the path if it is a directory. "),
ENTRY(IN_DONT_FOLLOW, "Do not follow a sym link. "),
ENTRY(IN_MASK_ADD, "Add to the mask of an already existing watch. "),
ENTRY(IN_ISDIR, "Event occurred against dir. "),
ENTRY(IN_ONESHOT, "Only send event once. "),
#undef ENTRY
};

int main(int argc, char *argv[])
{
	if (2 != argc) {
		usage(argv[0]);
		return 1;
	}
	const char *filepath = argv[1];

	const int fd = inotify_init();
	if (-1 == fd) {
		perror("inotify_init()");
		return 1;
	}

	const int wd = inotify_add_watch(fd, filepath, IN_ALL_EVENTS);
	if (-1 == wd) {
		perror("inotify_add_watch()");
		return 1;
	}

	while (1) {
		char buf[1024];
		int ret = read(fd, buf, sizeof(buf));
		if (-1 == ret) {
			perror("read()");
			return 1;
		} else if (1024 == ret) {
			fprintf(stderr, "Too small watch buffer\n");
			return 1;
		}

		/*inotify*/
		struct inotify_event *ie = (struct inotify_evnet *)buf;
		assert(wd == ie->wd);

		unsigned long bit;
		for_each_bit(bit, ie->mask) {
			int i;
			for (i = 0; i < sizeof(flag_descs)/sizeof(flag_descs[0]); i++)
				if (flag_descs[i].flag == (1 << bit))
					printf("%s\n", flag_descs[i].desc);
		}
	}

	return 0;
}
