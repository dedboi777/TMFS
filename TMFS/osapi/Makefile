flags=-O2 -Wall -std=c23 -I.
soflags=-ldl -shared -fPIC
ldflags=

.PHONY: clean

all: clean osapi.so osutil

osapi.so: osapi.o disk.o mystd.o fs.o
	cc $(flags) $(CFLAGS) $(soflags) $^ -o $@

osapi.o: osapi.c osapi.h os.h
	cc $(flags) $(CFLAGS) $(soflags) -c $<

disk.o: disk.c disk.h
	cc $(flags) $(CFLAGS) $(soflags) -c $<

mystd.o: mystd.c mystd.h
	cc $(flags) $(CFLAGS) $(soflags) -c $<

fs.o: fs.c fs.h
	cc $(flags) $(CFLAGS) $(soflags) -c $<

osutil: osutil.o osapi.o disk.o mystd.o fs.o
	cc $(flags) $(CFLAGS) $^ -o $@
	mv -f osutil ../util

osutil.o: osutil.c
	cc $(flags) $(CFLAGS) $(soflags) -c $<
	
clean:
	rm -f *.so *.o