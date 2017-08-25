CC = gcc
CFLAGS = -O0 -I../include -I../../c-libp2p/include -std=c99

ifdef DEBUG
CFLAGS += -g3
endif

DEPS = 
OBJS = init.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJS)
	cd config; make all;
	cd fsrepo; make all;
clean:
	rm -f *.o
	cd config; make clean;
	cd fsrepo; make clean;
