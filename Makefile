all: inotifier faccessor data

data:
	dd if=/dev/zero of=./data bs=4096 count=1

inotifier: inotifier.c
	gcc -Wall -o $@ $<

faccessor: faccessor.c
	gcc -Wall -o $@ $<

clean:
	-rm data
	-rm inotifier
	-rm faccessor

.PHONY: all clean
