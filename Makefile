default: scsniff

DEPS = atr.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

clean:
	rm -f scsniff *.o

scsniff: scsniff.o atr.o
	$(CC) -o $@ $^
