# By Panda.Guo at 2011-6-23
src = $(wildcard *.c)
obj = $(patsubst %.c, %.o, $(src))
target = ftp
# COMPILER_PREFIX = arm-linux-
CC = $(COMPILER_PREFIX)gcc
CFLAGS = -Wall -g -c 
LDFLAGS = -Wall -g 
all: $(target)
$(target): $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)
%.o : %.c 
	$(CC) $(CFLAGS) -o $@ $<
clean:
	-rm -f $(obj)
	-rm -f $(target)
.PHONY: clean all
