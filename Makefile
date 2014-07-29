VERSION	= 0.2
CC	= gcc
CFLAGS	= -Wall -shared -fPIC -ggdb

all: cfe_profiler34.so cfe_profiler35.so cfe_profiler36.so

%.so: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f cfe_profiler34.so cfe_profiler35.so cfe_profiler36.so
