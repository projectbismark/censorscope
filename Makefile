CC=gcc
OBJS=dns.o main.o sandbox.o register.o scheduling.o
EXE=censorscope
CFLAGS=`pkg-config lua5.1 --cflags` -g -Wall -std=gnu99
LDFLAGS=`pkg-config lua5.1 --libs` -lldns

LIBTOOL=libtool --tag=CC

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

censorscope: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(EXE)

all: censorscope

clean:
	rm -f $(OBJS)

clobber: clean
	rm $(EXE)
