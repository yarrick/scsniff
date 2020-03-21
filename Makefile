default: scsniff

DEPS = atr.h session.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

clean:
	rm -f scsniff *.o

scsniff: scsniff.o atr.o session.o
	$(CC) -o $@ $^
