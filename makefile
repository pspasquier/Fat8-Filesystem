CC     = gcc
CFLAGS = -Wall -std=c11 -Iinclude

SRCS = src/main.c src/disk_io.c src/cluster.c src/directory.c src/commands.c

all:
	$(CC) $(CFLAGS) -o fs $(SRCS)

clean:
	rm -f fs
