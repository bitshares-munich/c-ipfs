CC = gcc
CFLAGS = -O0 -I../../include -I../../../c-libp2p/include -I../../../c-multiaddr/include -I../../../c-protobuf -Wall

ifdef DEBUG
CFLAGS += -g3
endif

LFLAGS =
DEPS = 
OBJS = init.o main.o ../../commands/argument.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ipfs: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)
	
all: $(OBJS)

clean:
	rm -f *.o
	rm -f ipfs