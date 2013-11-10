CC=gcc
OBJS=main.o sandbox.o scheduling.o
EXE=censorscope
CFLAGS=`pkg-config lua5.1 --cflags` -g -Wall -std=gnu99
LDFLAGS=`pkg-config lua5.1 --libs`

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

#all:
#	$(LIBTOOL) --mode=compile cc $(CFLAGS) -c dns.c
#	$(LIBTOOL) --mode=link cc -rpath /usr/local/lib/lua5.1 $(LDFLAGS) -o libdns.la dns.lo
#	mv .libs/libdns.so.0.0.0 dns.so
