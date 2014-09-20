# Investigation of inotify(7)

Execute "make all", which generates the following 3 files.

- inotifier : A program that detects and reports all inotify events on a target file.
- faccessor : A program that Opens and accesses a target file via several different ways.
- data : The target file for the above two programs, which is a binary 4KB zero-filled file.
