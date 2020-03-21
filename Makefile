default: scsniff

DEPS = atr.h session.h pps.h data.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

clean:
	rm -f scsniff *.o

scsniff: scsniff.o atr.o session.o pps.o data.o
	$(CC) -o $@ $^
