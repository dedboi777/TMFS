flags=-O2 -Wall -std=c23 -I../FSAPI
ldflags=

.PHONY: clean clean-FSAPI

all: clean clean-FSAPI execute.com

../FSAPI/FSAPI.so: ../FSAPI/Hfunc.c ../FSAPI/Hfunc.h ../FSAPI/DiskEm.c ../FSAPI/DiskEm.h
	$(MAKE) -C ../FSAPI -sk "CFLAGS=$(flags)"
	
execute.com: execute.o ../FSAPI/FSAPI.so
	cc $(flags) $^ -o $@ $(ldflags)

execute.o: execute.c execute.h
	cc $(flags) -c $<

clean-FSAPI:
	$(MAKE) -C ../FSAPI clean

clean:
	rm -f *.o execute.com