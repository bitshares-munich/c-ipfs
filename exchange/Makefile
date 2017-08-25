CC = gcc
CFLAGS = -O0 -I../include -I../../c-libp2p/include -std=c99

ifdef DEBUG
CFLAGS += -g3
endif

DEPS = 
OBJS = 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJS)
	cd bitswap; make all;
clean:
	rm -f *.o
	cd bitswap; make clean;
