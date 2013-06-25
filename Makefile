VERSION	= 0.1
CC	= gcc
CFLAGS	= -Wall -shared -fPIC -ggdb

cfe_profiler.so: cfe_profiler.c
	$(CC) $(CFLAGS) cfe_profiler.c -o $@

clean:
	rm -f cfe_profiler.so
