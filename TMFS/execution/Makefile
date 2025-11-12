flags=-O2 -Wall -std=c23 -I../FSAPI
ldflags=

.PHONY: clean clean-FSAPI

all: clean clean-FSAPI execution.com

../FSAPI/FSAPI.so: ../FSAPI/Hfunc.c ../FSAPI/Hfunc.h ../FSAPI/DiskEm.c ../FSAPI/DiskEm.h
	$(MAKE) -C ../FSAPI -sk "CFLAGS=$(flags)"
	
execution.com: execution.o ../FSAPI/FSAPI.so
	cc $(flags) $^ -o $@ $(ldflags)

execution.o: execution.c execution.h
	cc $(flags) -c $<

clean-FSAPI:
	$(MAKE) -C ../FSAPI clean

clean:
	rm -f *.o execution.com