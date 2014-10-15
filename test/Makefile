CC := arm-none-linux-gnueabi-gcc
LD := arm-none-linux-gnueabi-gcc
CFLAGS := -g -Wall -Werror -static
LDFLAGS := -static

OBJECTS := prinfo.o stress.o

all: def_test main_test creator signaler waiter destroyer syscall-test

: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f def_test def_test.o 
	rm -f main_test main_test.o
	rm -f creator creator.o
	rm -f signaler signaler.o
	rm -f waiter waiter.o
	rm -f destroyer destroyer.o
	rm -f syscall-test syscall-test.o

.PHONY: clean
