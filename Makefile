LIBTOOL=libtool --tag=CC
CFLAGS=$(pkg-config lua5.1 --cflags)
LDFLAGS=-lldns $(pkg-config lua5.1 --libs)

all:
	$(LIBTOOL) --mode=compile cc $(CFLAGS) -c dns.c
	$(LIBTOOL) --mode=link cc -rpath /usr/local/lib/lua5.1 $(LDFLAGS) -o libdns.la dns.lo
	mv .libs/libdns.so.0.0.0 dns.so
