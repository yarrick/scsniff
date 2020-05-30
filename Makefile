default: scsniff

DEPS = result.h atr.h session.h pps.h data.h
CFLAGS = -std=c99 -Wall -pedantic -Wtype-limits

%.o: %.c $(DEPS) Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f scsniff test *.o

scsniff: scsniff.o atr.o session.o pps.o data.o
	$(CC) -o $@ $^

test: atr.o atr_test.o data.o data_test.o test.o
	$(CC) -o $@ -lcheck $^
