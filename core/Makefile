CC = gcc
CFLAGS = -O0 -I../include -I../../c-libp2p/include -I../../c-multiaddr/include -I../../c-protobuf -Wall -std=c99

ifdef DEBUG
CFLAGS += -g3
endif

LFLAGS = 
DEPS = builder.h ipfs_node.h
OBJS = builder.o daemon.o null.o ping.o bootstrap.o ipfs_node.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJS)

clean:
	rm -f *.o
