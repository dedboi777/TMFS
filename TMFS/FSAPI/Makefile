flags=-O2 -Wall -std=c23 -I.
soflags=-ldl -shared -fPIC
ldflags=

.PHONY: clean

all: clean FSAPI.so 

FSAPI.so: Hfunc.o DiskEm.o Mystd.o FileSystem.o
	cc $(flags) $(CFLAGS) $(soflags) $^ -o $@

Hfunc.o: Hfunc.c Hfunc.h Head.h
	cc $(flags) $(CFLAGS) $(soflags) -c $<

DiskEm.o: DiskEm.c DiskEm.h
	cc $(flags) $(CFLAGS) $(soflags) -c $<

Mystd.o: Mystd.c Mystd.h
	cc $(flags) $(CFLAGS) $(soflags) -c $<

FileSystem.o: FileSystem.c FileSystem.h
	cc $(flags) $(CFLAGS) $(soflags) -c $<

clean:
	rm -f *.so *.o