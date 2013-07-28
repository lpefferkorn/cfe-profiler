VERSION	= 0.1
CC	= gcc
CFLAGS	= -Wall -shared -fPIC -ggdb

all: cfe_profiler34.so cfe_profiler35.so

cfe_profiler34.so: cfe_profiler34.c
	$(CC) $(CFLAGS) cfe_profiler34.c -o $@

cfe_profiler35.so: cfe_profiler35.c
	$(CC) $(CFLAGS) cfe_profiler35.c -o $@

clean:
	rm -f cfe_profiler34.so cfe_profiler35.so
