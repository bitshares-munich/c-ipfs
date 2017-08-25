CC = gcc
CFLAGS = -O0 -I../include -I../../c-libp2p/include -I../../c-multihash/include -I../../c-multiaddr/include -I../../c-protobuf -Wall -std=c99

ifdef DEBUG
CFLAGS += -g3
endif

LFLAGS = 
DEPS = 
OBJS = importer.o exporter.o resolver.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJS)

clean:
	rm -f *.o
